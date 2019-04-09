#include "main.h"

static void get_thread_statistics(runner_data *, actions_stats_data *);
char * create_table_data(actions_stats_data *statistics);
static DWORD WINAPI print_table(LPVOID);
static DWORD WINAPI listen_input_commands(LPVOID feedback_object);

zlog_categories * loggers;
actions_stats_data statistics = {.statistics = NULL, .count = 0};

int main(int argc, char ** argv){	
	if(argc == 1){
		print_help();
		return 0;
	}
	char * test_plan_file_name = argv[1];

	if(logger_init() == 1){
		fprintf(stderr, "Failed to initialize logger\n");
		return 0;
	}

	loggers = logger_get_loggers();

	runner_data * r_data = generate_runner_data(test_plan_file_name);
	if(!r_data){
		printf("generate_runner_data returned NULL\n");
		return 0;
	}
	HANDLE hcontroller_thread = CreateThread(NULL, 0, test_controller, r_data, 0, NULL);
	if(!hcontroller_thread){
		free_runner_data(r_data);
		printf("Failed to create controller thread. [%lu]\n", GetLastError());
		return 1;
	}
	printf("Controller started successfully\n");
	_Atomic int stop_execution = 0;
	HANDLE hinput_listener_thread = CreateThread(NULL, 0, listen_input_commands, &stop_execution, 0, NULL);
	if(!hinput_listener_thread){
		printf("Failed to create input listener thread. [%lu]\n", GetLastError());
		return 1;
	}	
	
	// Run table printing in separate thread.
	HANDLE hprint_data_table_thread = CreateThread(NULL, 0, print_table, r_data, 0, NULL);
	if(!hprint_data_table_thread){
		printf("Failed to create print thread. [%lu]\n", GetLastError());
		return 1;
	}
	// This indicated that all test steps are done by test controller.
	WaitForSingleObject(hcontroller_thread, INFINITE);
	// Need to wait remaining threads to finish.
	//WaitForMultipleObjects();
	free_runner_data(r_data);
	logger_close();
	return 0;
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
		zlog_error(loggers->common, "Failed to allocate memory forstatistics->statistics structures array");
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
			DWORD result = 0;
			if(r_data->actions[i].threads[t].handle == NULL){
				(statistics->statistics)[i].pending_run++;
				continue;
			}
			if(!GetExitCodeThread(r_data->actions[i].threads[t].handle, &result)){
				zlog_error(loggers->common, "Failed to get thread exit code for action %s, thread # %u", (statistics->statistics)[i].name, t);
				(statistics->statistics)[i].failed++;
				continue;
			}
			if(result == STILL_ACTIVE){
				if(!r_data->actions[i].threads[t].stop_thread){
					(statistics->statistics)[i].running++;
				}
				else{
					(statistics->statistics)[i].pending_stop++;
				}
				continue;
			}
			else{
				(statistics->statistics)[i].stopped++;
			}
		}
	}
}

static DWORD WINAPI print_table(LPVOID data_pointer){
	runner_data * r_data = data_pointer;
	SetConsoleTitle("LoadTestingTool");
	HANDLE h_stdout, h_stdin;
	WORD w_old_color_attrs;
	CONSOLE_SCREEN_BUFFER_INFO csbInfo;

	h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	h_stdin = GetStdHandle(STD_INPUT_HANDLE);
	if(h_stdout == INVALID_HANDLE_VALUE){
		printf("Failed to receive stdout handle.");
		return 1L;
	}
	if(h_stdin == INVALID_HANDLE_VALUE){
		printf("Failed to receive stdin handle");
		return 1L;
	}

	if(!GetConsoleScreenBufferInfo(h_stdout, &csbInfo)){
		printf("Failed to get console screen buffer.");
		return 1L;
	}

	w_old_color_attrs = csbInfo.wAttributes;

	if(!SetConsoleTextAttribute(h_stdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY)){
		printf("Failed to set console text attribute.");
		return 1L;
	}

	DWORD old_mode;

	if(!GetConsoleMode(h_stdin, &old_mode)){
		printf("Failed to get console mode.");
		return 1L;
	}
	DWORD new_mode = old_mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
	if(!SetConsoleMode(h_stdin, new_mode)){
		printf("Failed to set new console mode");
		return 1L;
	}

	while(1){
		system("cls");
		get_thread_statistics(r_data, &statistics);
		const void * data_buffer = create_table_data(&statistics);
		DWORD num_of_chars = strlen(data_buffer);
		DWORD num_of_chars_written = 0;
		if(!WriteConsole(h_stdout, data_buffer, num_of_chars, &num_of_chars_written, NULL)){
			printf("Failed to write console.");
			break;
		}	
		Sleep(3000);	
		// Catch SIGTERM or similar signals to exit the loop
		/*
		if(SIGTERM){
			system("cls");
			break;
		}
		*/
	}
	if(!SetConsoleTextAttribute(h_stdout, w_old_color_attrs)){
		printf("Failed to set console text attribute.");
		return 1L;
	}
	if(!SetConsoleMode(h_stdin, old_mode)){
		printf("Failed to set old console mode.");
		return 1L;
	}
	return 0L;
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

static DWORD WINAPI listen_input_commands(LPVOID feedback_object){
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
