#include "main.h"
#include <cdk/cdk.h>

static void get_thread_statistics(runner_data *, actions_stats_data *);
char * create_table_data(actions_stats_data *statistics);
static void * print_table(void *);
//static DWORD WINAPI listen_input_commands(LPVOID feedback_object);

actions_stats_data statistics = {.statistics = NULL, .count = 0};

int main(int argc, char ** argv){	
	if(argc == 1){
		print_help();
		return EXIT_SUCCESS;
	}
	char * test_plan_file_name = argv[1];

	runner_data * r_data = generate_runner_data(test_plan_file_name);
	if(!r_data){
		error_message("generate_runner_data returned NULL\n");
		return EXIT_SUCCESS;
	}
	pthread_t hcontroller_thread;
	
	if(pthread_create(&hcontroller_thread, NULL, test_controller, r_data)){
		free_runner_data(r_data);
		error_message("Failed to create controller thread. [%s]\n", strerror(errno));
		return EXIT_FAILURE;
	}
	info_message("Controller started successfully\n");
	//_Atomic int stop_execution = 0;
	/*HANDLE hinput_listener_thread = CreateThread(NULL, 0, listen_input_commands, &stop_execution, 0, NULL);
	if(!hinput_listener_thread){
		printf("Failed to create input listener thread. [%lu]\n", GetLastError());
		return EXIT_FAILURE;
	}	*/
	
	// Run table printing in separate thread.
	pthread_t hprint_data_table_thread;
	
	if(pthread_create(&hprint_data_table_thread, NULL, print_table, r_data) != 0){
		error_message("Failed to create print thread. [%s]\n", strerror(errno));
		return EXIT_FAILURE;
	}
	// This indicates that all test steps are done by test controller.
	pthread_join(hprint_data_table_thread, NULL);
	
	pthread_join(hcontroller_thread, NULL);
	// Need to wait remaining threads to finish.
	//WaitForMultipleObjects();
	free_runner_data(r_data);
	return EXIT_SUCCESS;
}

void print_help(){
	printf("Please give test-plan file name as first parameter.\n");
}

static void get_thread_statistics(runner_data * r_data, actions_stats_data *statistics){
	unsigned int actions_count = r_data->actions_count;
	if(statistics->statistics == NULL){
		statistics->statistics = (action_stats_data*)malloc(sizeof(action_stats_data) * actions_count + 1);
	}
	if(statistics->statistics == NULL){
		error_message("Failed to allocate memory forstatistics->statistics structures array");
		return;
	}
	statistics->count = actions_count;
	for(unsigned int i = 0; i < actions_count; i++){
		statistics->statistics[i].thr_count = r_data->actions[i].threads_count;
		statistics->statistics[i].thr_run = r_data->actions[i].running_threads;
		statistics->statistics[i].running = 0;
		statistics->statistics[i].pending_run = 0;
		statistics->statistics[i].stopped = 0;
		statistics->statistics[i].pending_stop = 0;
		statistics->statistics[i].failed = 0;
		statistics->statistics[i].name = r_data->actions[i].name;

		for(unsigned int t = 0; t < (statistics->statistics)[i].thr_count; t++){
			int thd_status = 0;
			if(r_data->actions[i].threads[t].handle == 0){
				(statistics->statistics)[i].pending_run++;
				continue;
			}
			thd_status = pthread_tryjoin_np(r_data->actions[i].threads[t].handle, NULL);

			switch (thd_status){
				case 0:
					// The thread was stopped
					(statistics->statistics)[i].stopped++;
					break;
				case ESRCH:
				case EINVAL:
					error_message("Failed to get thread state. Errno: %i. Action %s, thread # %u",thd_status, (statistics->statistics)[i].name, t);
					(statistics->statistics)[i].failed++;
					break;
				case EBUSY:
					if(!r_data->actions[i].threads[t].stop_thread){
						(statistics->statistics)[i].running++;
					}
					else{
						(statistics->statistics)[i].pending_stop++;
					}
					break;
				default:
					break;			
			}
		}
	}
}

#define NAME_SIZE 50

static void * print_table(void * data_pointer){
	runner_data * r_data = data_pointer;
	actions_stats_data t_stats = {.statistics = NULL, .count = 0};	

	CDKSCREEN *cdkscreen	= 0;
	CDKMATRIX *cdkmatrix	= 0;
	const char * title 		= "<C>Load testing tool\n<C><#LT><#HL(30)><#RT>";
	int rows				= 5;
	int cols				= 8;
	int vrows				= 5;
	int vcols				= 8;

	bool use_coltitles = TRUE;
	
	const char * coltitle[NAME_SIZE];
	const char * rowtitle[NAME_SIZE];

	int colwidth[NAME_SIZE];
	int colvalue[NAME_SIZE];

	int col_spacing = 1;
	int row_spacing = 1;
	CDK_PARAMS params;
	char * argv = "programm.exe";
	CDKparseParams(1, &argv, &params, "trcT:C:R:" CDK_MIN_PARAMS);
	params.Shadow = !params.Shadow;
	use_coltitles = !CDKparamValue (&params, 'c', FALSE);
	col_spacing = CDKparamNumber2 (&params, 'C', -1);
	row_spacing = CDKparamNumber2 (&params, 'R', -1);
	cdkscreen = initCDKScreen(NULL);
	initCDKColor();

#define set_col(n, width, string) \
	coltitle[n] = use_coltitles ? string : 0 ;\
	colwidth[n] = width ;\
	colvalue[n] = vUMIXED

	set_col(1, 20, "</B/5>Name");
	set_col(2, 5, "</B/33>Pending");
	set_col(3, 5, "</B/33>Running");
	set_col(4, 5, "</B/33>Stopping");
	set_col(5, 5, "</B/33>Stoped");
	set_col(6, 5, "</B/33>Failed");
	set_col(7, 5, "</B/33>Total");
	set_col(8, 5, "</B/33>Running");

	cdkmatrix = newCDKMatrix(cdkscreen,
		CDKparamValue (&params, 'X', CENTER),
		CDKparamValue (&params, 'Y', CENTER),
		rows, cols, vrows, vcols,
		title,
		(CDK_CSTRING2)rowtitle,
		(CDK_CSTRING2)coltitle,
		colwidth, colvalue,
		col_spacing, row_spacing, ' ',
		COL,
		params.Box,
		params.Box,
		params.Shadow);

	if(cdkmatrix == 0){
		destroyCDKScreen(cdkscreen);
		endCDK();

		printf("Cannot create the matrix widget.\n");
		printf("Is the window too small?\n");
		printf("Programm will continue\n");
		return NULL;
	}

	while(1){
		get_thread_statistics(r_data, &t_stats);
		//CDK_CONST char * stats = create_table_data(&t_stats);
		char buf[NAME_SIZE];
		for(int k =  0; k < t_stats.count; k++){
			wcstombs(buf, t_stats.statistics[k].name, wcslen(t_stats.statistics[k].name));			
			setCDKMatrixCell(cdkmatrix, k + 1, 1, buf);
			sprintf(buf, "%u", t_stats.statistics[k].pending_run);
			setCDKMatrixCell(cdkmatrix, k + 1, 2, buf);
			sprintf(buf, "%u", t_stats.statistics[k].running);
			setCDKMatrixCell(cdkmatrix, k + 1, 3, buf);
			sprintf(buf, "%u", t_stats.statistics[k].pending_stop);
			setCDKMatrixCell(cdkmatrix, k + 1, 4, buf);
			sprintf(buf, "%u", t_stats.statistics[k].stopped);
			setCDKMatrixCell(cdkmatrix, k + 1, 5, buf);
			sprintf(buf, "%u", t_stats.statistics[k].failed);
			setCDKMatrixCell(cdkmatrix, k + 1, 6, buf);
			sprintf(buf, "%u", t_stats.statistics[k].thr_count);
			setCDKMatrixCell(cdkmatrix, k + 1, 7, buf);
			sprintf(buf, "%u", t_stats.statistics[k].thr_run);
			setCDKMatrixCell(cdkmatrix, k + 1, 8, buf);
		}
		drawCDKMatrix(cdkmatrix, FALSE);
		sleep(2);
	}
	return NULL;
}

char * create_table_data(actions_stats_data *statistics){
	char * output = NULL;
	char * next_row = NULL;
	int output_size = 320;
	const int row_len = 160;	

	output = (char*)malloc(output_size);
	if(output == NULL){
		printf("Failed to allocate memory for table.");
		return NULL;
	}
	sprintf(output,
	 "#    name                 pending_run    running    pending_stop    stopped    failed    total count    tot running\n");
	next_row = &output[strlen(output)];
	char * template = "%-5i|%-20.20s|%-14u|%-10u|%-15u|%-10u|%-9u|%-14u|%-10u";
	for(int i = 0; i < statistics->count; i++){
		sprintf(next_row, template,
		 i + 1,
		statistics->statistics[i].name,
		statistics->statistics[i].pending_run,
		statistics->statistics[i].running,
		statistics->statistics[i].pending_stop,
		statistics->statistics[i].stopped,
		statistics->statistics[i].failed,
		statistics->statistics[i].thr_count,
		statistics->statistics[i].thr_run);
		 if(output_size <= row_len * (i + 1)){
			 char * temp = (char *)realloc(output, row_len * (i + 1));
			 if(temp == NULL){
				 printf("Failed to reallocate memory for.");
				 break;
			 }
			 output = temp;
		 }
		next_row = &next_row[row_len];
	}
	return output;
}

/*static DWORD WINAPI listen_input_commands(LPVOID feedback_object){
	char input;
	while(1){
		input = getchar();
		switch(input){
			case 'q':
			*((int*)feedback_object) = 1;
			break;
			//case 'p':
			//pause
		}
	}
	return 0L;
}
*/
/*
static int signals_handler(int sig, int sig_subcode){
	PVOID exception_handler = AddVectoredExceptionHandler(1, vector_exception_handler);
	SetUnhandledExceptionFilter(NULL);
	return 0;
}

static LONG WINAPI vector_exception_handler(struct _EXCEPTION_POINTERS * _exception+info){
	if(!execution_context)
}*/