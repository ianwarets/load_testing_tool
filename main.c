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

}
int main(int argc, char ** argv){
	
	int rc = zlog_init("zlog.conf");
	if(rc){
		fprintf(stderr, "init failed\n");
		return -1;
	}
	
	zlog_programm_category = zlog_get_category("programm_log");
	zlog_statistics = zlog_get_category("statistics");
	if(!zlog_programm_category || !zlog_statistics){
		if(!zlog_statistics){
		fprintf(stderr, "get \"statistics\" zlog category failed/");
		}
		if(!zlog_programm_category){
			fprintf(stderr, "get \"programm_log\" zlog category failed\n");
		}
		zlog_fini();
		return -2;
	}
	time_t start_time;
	time(&start_time);
	struct tm * lt = localtime(&start_time);
	zlog_info(zlog_programm_category, "---------------------------------------------");
	zlog_info(zlog_programm_category, "Start date: %i/%02i/%02i %02i:%02i:%02i", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday + 1, lt->tm_hour, lt->tm_min, lt->tm_sec);
	init_http_requests(zlog_programm_category, zlog_statistics);	
	char * file_name = "etsy_links.txt";
	FILE * links_file = fopen(file_name, "rb");
	if(links_file == NULL){
		fprintf(stderr, "Failed to open file: %s\nExit/", file_name);
		return 1;
	}
	char ** url_array = NULL;
	size_t number_of_links = read_shop_links_from_file(links_file, &url_array);
	fclose(links_file);
	if( number_of_links == 0){
		fprintf(stdout, "No links found in file.\nExit.");		
		return 0;
	}
	long result;
	response_data_struct response = { .data = malloc(1), .size = 1 };
	for(int shop_iterator = 0; shop_iterator < number_of_links; shop_iterator++){
		fprintf(stdout, "----------------SHOP--------------\n\t%s\n", url_array[shop_iterator]);		
		result = get_request("Get links", url_array[shop_iterator], NULL, &response);
		while(result/100 == 3){
			char * redirect_link = get_redirect_link();
			result = get_request("Get links redirect", redirect_link, NULL, &response);
			if(result == -1){
				break;
			}
		}
		char ** href_array = NULL;
		size_t href_array_size = 0;
		if(result != 200){
			printf("\nIt's not OK\nCode=%li\n", result);

		}
		else{
			((char*)response.data)[response.size - 1] = '\0';	
			href_array_size = extract_listing_href((char*)response.data, &href_array);
		}	
		free(response.data);
		response.size = 1;
		response.data = malloc(1);

		for(int i = 0; i < href_array_size; i++){
			result = get_request("View listing", href_array[i], NULL, NULL);						
			if(result != 200){
				fprintf(stdout, "Failed to open \"%s\" Response code: %li\n", href_array[i], result);
			}
			else{
				fprintf(stdout, "%d. \"%s\", OK\n",i + 1, href_array[i]);
			}			
			free(href_array[i]);
		}
		free(href_array);
	}
	// Curl cleanup after all actions with handle.
	http_request_cleanup();
	zlog_fini();
	return 0;
}

/**
 *	Извлечение ссылок на товары
 */
size_t extract_listing_href(char * page_code, char *** href_array){
	char * image_tag = "data-palette-listing-image";
	char * href_begin = "https://www.etsy.com/listing/";
	char * href_end = "\"";
	int href_array_size = 10;

	*href_array = (char**)malloc(href_array_size * sizeof(char*));
	int i = 0;
	char * first_index = strstr(page_code, image_tag);
	if(first_index == NULL){
		free(*href_array);
	}
	while(first_index != NULL){
		first_index = strstr(first_index, href_begin);
		char * last_index = NULL;
		if(first_index == NULL){
			break;				
		}
		
		last_index = strstr(first_index, href_end);	
		if(last_index == NULL){
			break;
		}
		int size = last_index - first_index;
		char * result = (char*)malloc(size + 1);
		result = strncpy(result, first_index, size);
		result[size] = '\0';
		if(i == href_array_size){
			href_array_size += 2 * href_array_size;
			*href_array = (char**)realloc(*href_array, href_array_size * sizeof(char*));
			if(href_array == NULL){
				fprintf(stderr, "extract_listing_href: realloc failed.\n");
				break;
			}
		}
		(*href_array)[i++] = result;		
		first_index = strstr(last_index, image_tag);
	}
	return i;
}

/**
 *	Извлечение ссылок на магазины.
 *	Возвращает количество ссылок.
 *	Массив ссылок возвращается в параметре.
 */
size_t read_shop_links_from_file(FILE * file, char *** url_array){
	if(file == NULL){
		return 0;
	}
	fseek(file, 0, SEEK_END);
	long f_size = ftell(file);
	rewind(file);
	char * file_content = (char*)malloc(f_size * sizeof(char));
	if(file_content == NULL){
		fprintf(stderr, "read_shop_links_from_file: malloc failed for file_content.\n");
		return 0;
	}
	size_t read_count = fread(file_content, 1, f_size, file);
	if(read_count != f_size){
		free(file_content);
		fprintf(stderr, "read_shop_links_from_file: fread failed.\n");
		return 0;
	}
	file_content[read_count] = '\0';
	char ** new_lines_array = (char**)malloc(100);
	if(new_lines_array == NULL){
		free(file_content);
		fprintf(stderr, "read_shop_links_from_file: malloc failed for new_lines_array.\n");
		return 0;
	}
	size_t data_length = strlen(file_content);
	size_t index = 0;
	int lines_count = 0;
	while(index < data_length){
		char * next_pointer = NULL;
		do{
			next_pointer = strpbrk(&file_content[index], "\n\r");			
		}
		while(next_pointer - &file_content[index++] < 1);
		lines_count++;
		index += next_pointer - &file_content[index] + 1;
	}	

	*url_array = (char**)malloc(lines_count * sizeof(char*));
	if(*url_array == NULL){
		free(file_content);
		fprintf(stderr, "read_shop_links_from_file: malloc failed for url_array.\n");
		return 0;
	}

	char * last = file_content;
	int n = 0;
	for(int i = 0; i < lines_count; i++){
		while(!isalpha(last[n++]));
		size_t size = strcspn(&last[--n], "\n\r\0");

		(*url_array)[i] = (char*)malloc((size + 1) * sizeof(char));
		if((*url_array)[i] == NULL){
			for(int m = 0; m < i; m++){
				free((*url_array)[m]);
			}
			free(file_content);
			free(*url_array);
			free(new_lines_array);
			fprintf(stderr, "read_shop_links_from_file: malloc failed for url_array[%d].\n", i);
			return 0;
		}
		(*url_array)[i] = (char*)memcpy((*url_array)[i], &file_content[n], size);
		(*url_array)[i][size] = '\0';
		n += ++size;
	}
	return lines_count;
}