#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include "http_requests.h"
#include <ctype.h>
#include <zlog.h>
#include "main.h"
#include "test_controller.h"

size_t save_response_data(void*, size_t, size_t, void *);
size_t extract_listing_href(char *, char ***);
size_t read_shop_links_from_file(FILE *, char ***);


int logger_init(zlog_categories * log_categories){
	zlog_category_t * zlog_programm_category;
	zlog_category_t * zlog_statistics;
	zlog_category_t * zlog_common;

	int rc = zlog_init("zlog.conf");
	if(rc){
		fprintf(stderr, "zlog init failde\n");
		return 1;
	}

	zlog_programm_category = zlog_get_category("programm_log");
	zlog_statistics = zlog_get_category("statistics");
	zlog_common = zlog_get_category("common");
	if(!zlog_programm_category || !zlog_statistics || !zlog_common){
		if(!zlog_statistics){
			fprintf(stderr, "Get \"statistics\" zlog category failed.\n");
		}
		if(!zlog_programm_category){
			fprintf(stderr, "Get \"programm_log\" zlog category failed.\n");
		}
		if(!zlog_common){
			fprintf(stderr, "Get \"common\" zlog category failed.\n");
		}
		zlog_fini();
		return 1;
	}

	log_categories->common = zlog_common;
	log_categories->programm_category = zlog_programm_category;
	log_categories->statistics = zlog_statistics;
	return 0;
}

int create_test_configuration_sturcture(char * test_file_data, runner_data * test_runner_data){
	test_runner_data->start_delay = 0;
	test_runner_data->steps_count = 2;
	test_runner_data->steps = (step_data*)malloc(sizeof(step_data) * test_runner_data-> steps_count);
	//test_runner_data->steps[0]
	return 0;
}

step_data * generate_steps_data(int count){
	step_data * sd = (step_data*)malloc(sizeof(step_data) * count);
	//sd[0].next_step = sd[1];
	//sd->
	return NULL;
}

int main(int argc, char ** argv){
	
	return 0;
}
