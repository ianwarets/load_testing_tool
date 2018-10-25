#include "main.h"

static int get_thread_statistics(runner_data *, stats_data **);
char * create_table_data(stats_data * statistics, int);
void print_table(runner_data * r_data);

zlog_categories * loggers;

int main(int argc, char ** argv){	
	if(argc == 1){
		print_help();
	}
	char * test_plan_file_name = argv[1];

	if(logger_init() == 1){
		return 0;
	}

	loggers = logger_get_loggers();

	runner_data * r_data = generate_runner_data(test_plan_file_name);
	if(!r_data){
		printf("generate_runner_data returned NULL\n");
		return 0;
	}
	HANDLE controller_thread = CreateThread(NULL, 0, test_controller, r_data, 0, NULL);
	if(!controller_thread){
		free_runner_data(r_data);
		printf("Failed to create controller thread. [%lu]\n", GetLastError());
		return 1;
	}
	printf("Controller started successfully\n");
	
	print_table(r_data);
	
	free_runner_data(r_data);
	logger_close();
	return 0;
}

void print_help(){
	printf("Для запуска теста необходимо указать имя тест-плана.\n");
}

static int get_thread_statistics(runner_data * r_data, stats_data ** statistics){
	unsigned int a_count = r_data->actions_count;
	if(*statistics == NULL){
		*statistics = (stats_data*)malloc(sizeof(stats_data) * a_count + 1);
	}
	if(*statistics == NULL){
		zlog_error(loggers->common, "Failed to allocate memory for statistics structures array");
		return 0;
	}
	unsigned int i;
	for(i = 0; i < a_count; i++){
		(*statistics)[i].thr_count = r_data->actions[i].threads_count;
		(*statistics)[i].thr_run = r_data->actions[i].running_threads;
		(*statistics)[i].running = 0;
		(*statistics)[i].pending_run = 0;
		(*statistics)[i].stopped = 0;
		(*statistics)[i].pending_stop = 0;
		(*statistics)[i].failed = 0;
		(*statistics)[i].name = r_data->actions[i].name;

		for(unsigned int t = 0; t < (*statistics)[i].thr_count; t++){
			DWORD result = 0;
			if(r_data->actions[i].threads[t].handle == NULL){
				(*statistics)[i].pending_run++;
				continue;
			}
			if(!GetExitCodeThread(r_data->actions[i].threads[t].handle, &result)){
				zlog_error(loggers->common, "Failed to get thread exit code for action %s, thread # %u", (*statistics)[i].name, t);
				(*statistics)[i].failed++;
				continue;
			}
			if(result == STILL_ACTIVE){
				if(!r_data->actions[i].threads[t].stop_thread){
					(*statistics)[i].running++;
				}
				else{
					(*statistics)[i].pending_stop++;
				}
				continue;
			}
			else{
				(*statistics)[i].stopped++;
			}
		}
	}
	return i;
}

void print_table(runner_data * r_data){
	SetConsoleTitle("LoadTestingTool");
	HANDLE h_stdout, h_stdin;
	WORD w_old_color_attrs;
	CONSOLE_SCREEN_BUFFER_INFO csbInfo;

	h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	h_stdin = GetStdHandle(STD_INPUT_HANDLE);
	if(h_stdout == INVALID_HANDLE_VALUE){
		printf("Failed to receive stdout handle.");
		return;
	}
	if(h_stdin == INVALID_HANDLE_VALUE){
		printf("Failed to receive stdin handle");
		return;
	}

	if(!GetConsoleScreenBufferInfo(h_stdout, &csbInfo)){
		printf("Failed to get console screen buffer.");
		return;
	}

	w_old_color_attrs = csbInfo.wAttributes;

	if(!SetConsoleTextAttribute(h_stdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY)){
		printf("Failed to set console text attribute.");
		return;
	}

	DWORD old_mode;

	if(!GetConsoleMode(h_stdin, &old_mode)){
		printf("Failed to get console mode.");
		return;
	}
	DWORD new_mode = old_mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
	if(!SetConsoleMode(h_stdin, new_mode)){
		printf("Failed to set new console mode");
		return;
	}

	stats_data * statistics = NULL;

	while(1){
		system("cls");
		int count = get_thread_statistics(r_data, &statistics);
		const void * data_buffer = create_table_data(statistics, count);
		DWORD num_of_chars = strlen(data_buffer);
		DWORD num_of_chars_written = 0;
		if(!WriteConsole(h_stdout, data_buffer, num_of_chars, &num_of_chars_written, NULL)){
			printf("Failed to write console.");
			break;
		}	
		Sleep(300);	
	}
	if(!SetConsoleTextAttribute(h_stdout, w_old_color_attrs)){
		printf("Failed to set console text attribute.");
		return;
	}
	if(!SetConsoleMode(h_stdin, old_mode)){
		printf("Failed to set old console mode.");
		return;
	}
}

char * create_table_data(stats_data * statistics, int count){
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
	for(int i = 0; i < count; i++){
		char name[30] = "g_test_STUB";
		sprintf(next_row, template,
		 i + 1,
		 //statistics[i].name,s
		 name,
		 statistics[i].pending_run,
		 statistics[i].running,
		 statistics[i].pending_stop,
		 statistics[i].stopped,
		 statistics[i].failed,
		 statistics[i].thr_count,
		 statistics[i].thr_run);
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
