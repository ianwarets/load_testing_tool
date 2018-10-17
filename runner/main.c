
#include "main.h"

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
	
	SleepEx(INFINITE, TRUE);
	
	free_runner_data(r_data);
	logger_close();
	return 0;
}

void print_help(){
	printf("Для запуска теста необходимо указать имя тест-плана.\n");
}
