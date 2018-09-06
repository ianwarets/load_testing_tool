#include <curl/curl.h>
#include <string.h>
#include "http_requests.h"


static CURLcode http_request(CURL *, char*, struct curl_slist *, response_data_struct *);
static size_t save_response_data(void*, size_t, size_t, void *);
static void check_getinfo_result(CURLcode, curl_off_t *);

/*
	Выполнение GET запроса.
*/
CURLcode get_request(CURL * hCurl, char * url, struct curl_slist * headers, response_data_struct * callback_data){	
	curl_easy_setopt(hCurl, CURLOPT_HTTPGET, 1L);
	return http_request(hCurl, url, headers, callback_data);
}

/*
	Выполнение POST запроса.
*/
CURLcode post_request(CURL * hCurl, char * url, struct curl_slist * headers, response_data_struct * callback_data){
	curl_easy_setopt(hCurl, CURLOPT_HTTPPOST, 1L);
	return http_request(hCurl, url, headers, callback_data);
}

/*
	Сбор статистики по транзакции
*/
transaction_tech_data collect_request_statistics(CURL * hCurl){
	curl_off_t response_time;
	curl_off_t network_delay;
	curl_off_t uploaded_bytes;
	curl_off_t received_bytes;
	curl_off_t header_size;
	curl_off_t upload_speed;
	curl_off_t download_speed;

	// Returns CURLE_OK if the option is supported, and CURLE_UNKNOWN_OPTION if not. 
	check_getinfo_result(curl_easy_getinfo(hCurl, CURLINFO_TOTAL_TIME_T, &response_time),&response_time);
	// Время установления соединения.
	check_getinfo_result(curl_easy_getinfo(hCurl, CURLINFO_CONNECT_TIME_T, &network_delay),&network_delay);
	check_getinfo_result(curl_easy_getinfo(hCurl, CURLINFO_REQUEST_SIZE, &uploaded_bytes),&uploaded_bytes);
	// Объем полезных загруженных данных, не включает метаданные. 
	check_getinfo_result(curl_easy_getinfo(hCurl, CURLINFO_SIZE_DOWNLOAD_T, &received_bytes),&received_bytes);
	// Суммарный объем заголовков.
	check_getinfo_result(curl_easy_getinfo(hCurl, CURLINFO_HEADER_SIZE, &header_size),&header_size);
	check_getinfo_result(curl_easy_getinfo(hCurl, CURLINFO_SPEED_UPLOAD_T, &upload_speed),&upload_speed);
	check_getinfo_result(curl_easy_getinfo(hCurl, CURLINFO_SPEED_DOWNLOAD_T, &download_speed),&download_speed);
	
	transaction_tech_data tech_data_struct = {
		.response_time = response_time,
		.netw_delay = network_delay,
		.uploaded_bytes = uploaded_bytes,
		.received_bytes = received_bytes + header_size,
		.upload_speed = upload_speed,
		.download_speed = download_speed
	};
	return tech_data_struct;
}

/*
	Выполнение HTTP запроса по заданному адресу и заголовкам.
	request_hadle - переменная соединения
	url - адрес запроса,
	headers - заголовки запроса,
	callback_function - функция обратного вызова, которой передаются полученные данные,
	callback_data - один из параметров, передаваемых в функцию обратного вызова. Используется для получения данных ответа.
*/
static CURLcode http_request(CURL * request_handle, char * url, struct curl_slist * headers, response_data_struct * callback_data){
	char * user_agent = "libcurl";
	char * accept_encoding = "gzip, deflate, br";
	#ifdef DEBUG
	curl_easy_setopt(request_handle, CURLOPT_VERBOSE, 1L);
	#endif
	// Set URL
	curl_easy_setopt(request_handle, CURLOPT_URL, url);	
	curl_easy_setopt(request_handle, CURLOPT_USE_SSL, CURLUSESSL_TRY);	
	
	// Задание пути к файлу сертификатов.
	curl_easy_setopt(request_handle, CURLOPT_CAINFO, "cacert.pem");
	
	// Включение приема всех допустимых кодировок данных.
	curl_easy_setopt(request_handle, CURLOPT_ACCEPT_ENCODING, "");

	curl_easy_setopt(request_handle, CURLOPT_WRITEFUNCTION, save_response_data);
	curl_easy_setopt(request_handle, CURLOPT_WRITEDATA, callback_data);	

	// Задание заголовков запроса.
	if(headers != NULL){
		curl_easy_setopt(request_handle, CURLOPT_HTTPHEADER, headers);
	}
	else{
		curl_easy_setopt(request_handle, CURLOPT_USERAGENT, user_agent);
		curl_easy_setopt(request_handle, CURLOPT_ACCEPT_ENCODING, accept_encoding);
	}
	// Perform request.
	return curl_easy_perform(request_handle);
}

/*
	Функция для обработки полученных в результате запроса данных.
	Данные записываются в переменную userp в виде строки(массив символов)
	Сигнатура функции описана на сайте libcurl
*/
static size_t save_response_data(void* buffer, size_t size, size_t nmemb, void * userp){
	size_t result = size * nmemb;
	if(userp == NULL){
		return result;
	}
	if(result == 0){
		return 0;
	}
	size_t current_size = ((response_data_struct*)userp)->size;
	#ifdef DEBUG
	fprintf(stdout, "Current size of response_struct data: %u\n", current_size);
	fprintf(stdout, "Received data size: %u\n", result);
	#endif
	void ** response = &(((response_data_struct*)userp)->data);
	if(*response == NULL){
		*response = malloc(result + current_size);
	}
	else{
		*response = realloc(*response, result + current_size);	
	}
	if(response == NULL){
		fprintf(stderr, "save_response_data: memory allocation/reallocation failed\n");
		return 0;
	}
	void * response_last_byte = *response + current_size - 1;
	#ifdef DEBUG
	fprintf(stdout, "response pointer value : %p\nresponse_lase_byte pointer value: %p\n", response, response_last_byte);
	#endif
	memcpy(response_last_byte, buffer, result);
	if(*response == NULL){
		fprintf(stderr, "save_response_data: memcpy returned ERROR");
		return 0;
	}
	((response_data_struct*)userp)->size += result;
	return result;
}

/*
	Задает значение результата -1, если код = CURLE_UNKNOWN_OPTION
*/
static void check_getinfo_result(CURLcode code, curl_off_t * result){
	if(code == CURLE_UNKNOWN_OPTION){
		*result = -1;
	}
	return;
}