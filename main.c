#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include "http_requests.h"
#include <ctype.h>

size_t save_response_data(void*, size_t, size_t, void *);
size_t extract_listing_href(char *, char ***);
size_t read_shop_links_from_file(FILE *, char ***);
size_t result_callback_mock(void *, size_t, size_t, void *);

int main(int argc, char ** argv){
	CURLcode result;
	CURL * request_handle = curl_easy_init();	
	if(!request_handle){
		printf("Failed to create handle");
		return 0;
	}	
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
	
	response_data_struct response = { .data = malloc(1), .size = 1 };
	for(int shop_iterator = 0; shop_iterator < number_of_links; shop_iterator++){
		fprintf(stdout, "----------------SHOP--------------\n\t%s\n", url_array[shop_iterator]);
		result = get_request(request_handle, url_array[shop_iterator], NULL, &response);
		if(result != CURLE_OK){
			printf("\nIt's not OK\nCode=%d\n", result);		
		}
		else{
			long response_code;
			result = curl_easy_getinfo(request_handle, CURLINFO_RESPONSE_CODE, &response_code);
			if(result == CURLE_OK) {
				((char*)response.data)[response.size - 1] = '\0';									
			}
			else{
				fprintf(stderr, "Failed to get info from response. Result code: %i\n", result);					
			}
		}	
		char ** href_array = NULL;
		size_t href_array_size = extract_listing_href((char*)response.data, &href_array);
		free(response.data);
		response.size = 1;
		response.data = malloc(1);

		for(int i = 0; i < href_array_size; i++){
			result = get_request(request_handle, href_array[i], NULL, NULL);
			if(result != CURLE_OK){
				continue;
			}		
			long resp_code;
			result = curl_easy_getinfo(request_handle, CURLINFO_RESPONSE_CODE, &resp_code);			
			if(result == CURLE_OK){			
				if(resp_code != 200){
					fprintf(stdout, "Failed to open \"%s\" Response code: %li\n", href_array[i], resp_code);
				}
				else{
					fprintf(stdout, "%d, \"%s\", OK\n",i, href_array[i]);
				}
			}
			else{
				fprintf(stdout, "Could not get response code for \"%s\"\n", href_array[i] );
			}
			free(href_array[i]);
		}
		free(href_array);
	}
	// Curl cleanup after all actions with handle.
	curl_easy_cleanup(request_handle);
	return 0;
}

/*
	Извлечение ссылок на товары
*/
size_t extract_listing_href(char * page_code, char *** href_array){
	char * image_tag = "data-palette-listing-image";
	char * href_begin = "https://www.etsy.com/listing/";
	char * href_end = "\"";
	int href_array_size = 10;

	*href_array = (char**)malloc(href_array_size * sizeof(char*));
	int i = 0;
	char * first_index = strstr(page_code, image_tag);
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

/*
	Извлечение ссылок на магазины.
	Возвращает количество ссылок.
	Массив ссылок возвращается в параметре.
*/
size_t read_shop_links_from_file(FILE * file, char *** url_array){
	if(file == NULL){
		return 0;
	}
	fseek(file, 0, SEEK_END);
	long f_size = ftell(file);
	rewind(file);
	char * file_content = (char*)malloc(f_size);
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
	int lines_count = 0;
	char ** new_lines_array = (char**)malloc(100);
	if(new_lines_array == NULL){
		free(file_content);
		fprintf(stderr, "read_shop_links_from_file: malloc failed for new_lines_array.\n");
		return 0;
	}
	new_lines_array[lines_count] = strchr(file_content, '\n');
	while(new_lines_array[lines_count] != NULL){
		// Ограничение на количество участников.
		if(lines_count > 99){
			break;
		}
		// Проверка конца строки.
		if(new_lines_array[lines_count] == '\0'){
			break;
		}
		// Запись указателя на следующий найденный символ '\n' в элемент массива по текущему индексу. 
		// Точкой отсчета для поиска символа берется предыдущий элемент массива начиная со второго символа.
		lines_count++;
		new_lines_array[lines_count] = strchr(new_lines_array[lines_count - 1] + 1, '\n');
		
	}

	*url_array = (char**)malloc(lines_count * sizeof(char*));
	if(*url_array == NULL){
		free(file_content);
		fprintf(stderr, "read_shop_links_from_file: malloc failed for url_array.\n");
		return 0;
	}
	char * prev = file_content;
	for(int i = 0; i < lines_count; i++){		
		int n = 0;
		while(!isalpha(prev[n++]));
		char * first_alpha = &prev[--n];
		long size = new_lines_array[i] - first_alpha;
		(*url_array)[i] = (char*)malloc(size);
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
		(*url_array)[i] = (char*)memcpy((*url_array)[i], first_alpha, size);
		(*url_array)[i][size - 1] = '\0';
		prev = &new_lines_array[i][1];
	}
	return lines_count + 1;
}