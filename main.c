#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <zlog.h>
#include "main.h"
#include "test_plan.h"
#include "logger.h"

size_t save_response_data(void*, size_t, size_t, void *);
size_t extract_listing_href(char *, char ***);
size_t read_shop_links_from_file(FILE *, char ***);

void print_help();

int main(int argc, char ** argv){

	if(argc == 1){
		print_help();
	}
	char * test_plan_file_name = argv[1];

	if(!logger_init()){
		return 0;
	}

	runner_data * r_data = generate_runner_data(test_plan_file_name);
	if(!r_data){
		printf("generate_runner_data returned NULL");
		return 0;
	}
	HANDLE controller_thread = CreateThread(NULL, 0, test_controller, r_data, 0, NULL);
	if(!controller_thread){
		free_runner_data(r_data);
		printf("Failed to create controller thread. [%lu]", GetLastError());
	}
	printf("Controller started successfully");
	free_runner_data(r_data);
	return 0;
}

void print_help(){
	printf("Для запуска теста необходимо указать имя тест-плана.");
}
