#include "http_requests.h"
#include <string.h>
#include <time.h>
#include <logger.h>
#include <stdlib.h>


static long http_request(char *, char*, struct curl_slist *, response_data_struct *);
static size_t save_response_data(void*, size_t, size_t, void *);
static void save_request_statistics(char * name, struct timespec * time);
static long make_request(char * name, char * url, struct curl_slist * headers, response_data_struct * response_data);

__thread CURL * hCurl;

int init_http_requests(){
	hCurl = curl_easy_init();
	if(hCurl == NULL){
		fatal_message("Failed to create CURL handler.");
		return -1;
	}

	// Установка удержания соединения.
	curl_easy_setopt(hCurl, CURLOPT_TCP_KEEPALIVE, 1L);
	
	// Установка периода бездействия удержания 120 с.
	curl_easy_setopt(hCurl, CURLOPT_TCP_KEEPIDLE, 120L);
	
	// Установка периода проверки соединения 60 с.
	curl_easy_setopt(hCurl, CURLOPT_TCP_KEEPINTVL, 60L);
	
	return 0;
}

void http_requests_cleanup(){
	curl_easy_cleanup(hCurl);
}

long get_request(char * name, char * url, struct curl_slist * headers, response_data_struct * response_data){	
	curl_easy_setopt(hCurl, CURLOPT_HTTPGET, 1L);
	return make_request(name, url, headers, response_data);
	
}

long post_request(char * name, char * url, struct curl_slist * headers, response_data_struct * response_data, char * post_data, long post_data_size){
	if(curl_easy_setopt(hCurl, CURLOPT_POSTFIELDS, post_data) != CURLE_OK){
		error_message("Failed to set POST body value");
		return -1;
	}
	if(curl_easy_setopt(hCurl, CURLOPT_POSTFIELDSIZE, post_data_size) != CURLE_OK){
		error_message("Failed to set POST field size property");
		return -1;
	}
	return make_request(name, url, headers, response_data);
}

static long make_request(char * name, char * url, struct curl_slist * headers, response_data_struct * response_data){	
#ifdef DEBUG
	debug_message("URL: \"%s\", curl_slist has value: %c, response_data_struct has value: %c", url, (curl_slist == NULL) ? 'n' : 'y', (response_data == NULL) ? 'n' : 'y');
#endif
	long result =  http_request(name, url, headers, response_data);	
	return result;
}

/**
 *	Сбор и сохранение статистики по запросу.
 */
static void save_request_statistics(char * name, struct timespec * time){
#ifdef DEBUG
	debug_message("hCurl has value:%c", (hCurl == NULL) ? L'n' : 'y');
#endif

	curl_off_t response_time = -1;
	curl_off_t network_delay = -1;
	long uploaded_bytes = -1;
	curl_off_t received_bytes = -1;
	long header_size = -1;
	curl_off_t upload_speed = -1;
	curl_off_t download_speed = -1;
	
	// Returns CURLE_OK if the option is supported, and CURLE_UNKNOWN_OPTION if not. 
	curl_easy_getinfo(hCurl, CURLINFO_TOTAL_TIME_T, &response_time);

	// Время установления соединения.
	curl_easy_getinfo(hCurl, CURLINFO_CONNECT_TIME_T, &network_delay);

	curl_easy_getinfo(hCurl, CURLINFO_REQUEST_SIZE, &uploaded_bytes);

	// Объем полезных загруженных данных, не включает метаданные. 
	curl_easy_getinfo(hCurl, CURLINFO_SIZE_DOWNLOAD_T, &received_bytes);

	// Суммарный объем заголовков.
	curl_easy_getinfo(hCurl, CURLINFO_HEADER_SIZE, &header_size);

	curl_easy_getinfo(hCurl, CURLINFO_SPEED_UPLOAD_T, &upload_speed);
	
	curl_easy_getinfo(hCurl, CURLINFO_SPEED_DOWNLOAD_T, &download_speed);

	long time_v = (time->tv_sec * 1000) + time->tv_nsec / 1000000;
#ifdef DEBUG
	debug_message("\tCURLINFO_TOTAL_TIME_T : %li\n\
	CURLINFO_CONNECT_TIME_T : %li\n\
	CURLINFO_REQUEST_SIZE : %li\n\
	CURLINFO_SIZE_DOWNLOAD_T : %li\n\
	CURLINFO_HEADER_SIZE : %li\n\
	CURLINFO_SPEED_UPLOAD_T : %li\n\
	CURLINFO_SPEED_DOWNLOAD_T : %li\n\
	CURLINFO_PRIVATE : [time]:%li", 
	response_time,
	network_delay,
	uploaded_bytes,
	received_bytes,
	header_size,
	upload_speed,
	download_speed,
	time_v);
#endif
	info_message("%li,%s,%li,%li,%li,%li,%li,%li", 
			time_v,
			name,
			response_time, 
			network_delay, 
			uploaded_bytes, 
			received_bytes + header_size,
			upload_speed,
			download_speed);
}


/*
 *	Выполнение HTTP запроса по заданному адресу и заголовкам.
 *	url - адрес запроса,
 *	headers - заголовки запроса,
 *	callback_function - функция обратного вызова, которой передаются полученные данные,
 *	response_data - один из параметров, передаваемых в функцию обратного вызова. Используется для получения данных ответа.
 */
static long http_request(char * name, char * url, struct curl_slist * headers, response_data_struct * response_data){
	if(name == NULL){
			error_message("Request name is not set!");
			return -1;
	}

	static char * user_agent = "Медведь. Инструмент генерации нагрузки.";
	static char * cert_file_name = "cacert.pem";
#ifdef DEBUG
	curl_easy_setopt(hCurl, CURLOPT_VERBOSE, 1L);
#endif
#ifdef DEBUG
	debug_message("CURLOPT_URL: %s", url);
#endif
	// Set URL
	curl_easy_setopt(hCurl, CURLOPT_URL, url);	
	curl_easy_setopt(hCurl, CURLOPT_USE_SSL, CURLUSESSL_TRY);		
	// Задание пути к файлу сертификатов.
	curl_easy_setopt(hCurl, CURLOPT_CAINFO, cert_file_name);
	// Включение приема всех допустимых кодировок данных.
	curl_easy_setopt(hCurl, CURLOPT_ACCEPT_ENCODING, "");
	curl_easy_setopt(hCurl, CURLOPT_WRITEFUNCTION, save_response_data);
	curl_easy_setopt(hCurl, CURLOPT_WRITEDATA, response_data);	

	// Задание заголовков запроса.
	if(headers != NULL){
		curl_easy_setopt(hCurl, CURLOPT_HTTPHEADER, headers);
	}
	else{
		curl_easy_setopt(hCurl, CURLOPT_USERAGENT, user_agent);
	}
	struct timespec time_val;
	clock_gettime(CLOCK_REALTIME, &time_val);
	
	// Perform request.
	CURLcode result = curl_easy_perform(hCurl);
	
	if(result != CURLE_OK){
		return -1;
	} 
	save_request_statistics(name, &time_val);
	long response_code;
	result = curl_easy_getinfo(hCurl, CURLINFO_RESPONSE_CODE, &response_code);
	if(result != CURLE_OK){
		return -1;
	}
	return response_code;
}

/*
 *	Функция для обработки полученных в результате запроса данных.
 *	Данные записываются в переменную userp в виде строки(массив символов)
 *	Сигнатура функции описана на сайте libcurl
 */
static size_t save_response_data(void* buffer, size_t size, size_t nmemb, void * userp){
	size_t result = size * nmemb;
#ifdef DEBUG
	debug_message("size * nmemb = %u.", result);
#endif
	if(userp == NULL){
#ifdef DEBUG
		debug_message("User data pointer is NULL. Nothing to do.");
#endif
		return result;
	}
	if(result == 0){
#ifdef DEBUG
		debug_message("Result = 0. Nothing to do");
#endif
		return 0;
	}
	size_t current_size = ((response_data_struct*)userp)->size;
	
	void ** response = &(((response_data_struct*)userp)->data);
	
	*response = realloc(*response, result + current_size);	
	
	if(response == NULL){
		error_message("Memory allocation/reallocation failed.");
		return 0;
	}
	
	void * response_last_byte = *response;
	if(current_size > 1){		
		response_last_byte += current_size - 1;
	}
#ifdef DEBUG
	debug_message("Response pointer value: %p.\tresponse_last_byte %i", response_last_byte);
#endif
	memcpy(response_last_byte, buffer, result);
	if(*response == NULL){
		error_message("Failed to copy from [buffer] to [userp->data].");
		return 0;
	}
	((response_data_struct*)userp)->size += result;
	return result;
}

char * get_redirect_link(){
	char * redir_url = NULL;
	curl_easy_getinfo(hCurl, CURLINFO_REDIRECT_URL , &redir_url);
	return redir_url;
}