#include "http_requests.h"
#include <string.h>
#include <time.h>
#include <windows.h>
#include <logger.h>


static long http_request(char *, char*, struct curl_slist *, response_data_struct *);
static size_t save_response_data(void*, size_t, size_t, void *);
static void save_request_statistics(char * name);
static long make_request(char * name, char * url, struct curl_slist * headers, response_data_struct * response_data);

static zlog_categories * loggers;
__thread CURL * hCurl;

int init_http_requests(){
	if(loggers == NULL){
		loggers = logger_get_loggers();
	}
	hCurl = curl_easy_init();
	if(hCurl == NULL){
		zlog_fatal(loggers->scenario, "Failed to create CURL handler.");
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
		zlog_error(loggers->scenario, "Failed to set POST body value");
		return -1;
	}
	if(curl_easy_setopt(hCurl, CURLOPT_POSTFIELDSIZE, post_data_size) != CURLE_OK){
		zlog_error(loggers->scenario, "Failed to set POST field size property");
		return -1;
	}
	return make_request(name, url, headers, response_data);
}

static long make_request(char * name, char * url, struct curl_slist * headers, response_data_struct * response_data){	
#ifdef DEBUG
	zlog_debug(loggers->scenario, "URL: \"%s\", curl_slist has value: %c, response_data_struct has value: %c", url, (headers == NULL) ? 'n' : 'y', (response_data == NULL) ? 'n' : 'y');
#endif
	long result =  http_request(name, url, headers, response_data);	
	return result;
}

/**
 *	Сбор и сохранение статистики по запросу.
 */
static void save_request_statistics(char * name){
#ifdef DEBUG
	zlog_debug(loggers->scenario, "hCurl has value:%c", (hCurl == NULL) ? 'n' : 'y');
#endif

	curl_off_t response_time = -1;
	curl_off_t network_delay = -1;
	long uploaded_bytes = -1;
	curl_off_t received_bytes = -1;
	long header_size = -1;
	curl_off_t upload_speed = -1;
	curl_off_t download_speed = -1;
	time_t * time = NULL;	
	
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
	
	curl_easy_getinfo(hCurl, CURLINFO_PRIVATE, &time);
#ifdef DEBUG
	zlog_debug(loggers->scenario, "\tCURLINFO_TOTAL_TIME_T : %lli\n\
	CURLINFO_CONNECT_TIME_T : %lli\n\
	CURLINFO_REQUEST_SIZE : %li\n\
	CURLINFO_SIZE_DOWNLOAD_T : %lli\n\
	CURLINFO_HEADER_SIZE : %li\n\
	CURLINFO_SPEED_UPLOAD_T : %lli\n\
	CURLINFO_SPEED_DOWNLOAD_T : %lli\n\
	CURLINFO_PRIVATE : [time]:%li", 
	response_time,
	network_delay,
	uploaded_bytes,
	received_bytes,
	header_size,
	upload_speed,
	download_speed,
	time);
#endif
	char * message_template = "%li,%s,%lli,%lli,%li,%lli,%lli,%lli";
	zlog_info(loggers->statistics, 
			message_template, 
			*time,
			name,
			response_time, 
			network_delay, 
			uploaded_bytes, 
			received_bytes + header_size,
			upload_speed,
			download_speed);
	free(time);
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
			zlog_error(loggers->scenario, "Request name is not set!");
			return -1;
	}

	static char * user_agent = "Медведь. Инструмент генерации нагрузки.";
	static char * cert_file_name = "cacert.pem";
#ifdef DEBUG
		curl_easy_setopt(hCurl, CURLOPT_VERBOSE, 1L);
#endif
#ifdef DEBUG
	zlog_debug(loggers->scenario, "CURLOPT_URL: %s", url);
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
	time_t * time_val = (time_t*)malloc(sizeof(time_t));
	if(time_val == NULL){
		zlog_error(loggers->scenario, "Failed to allocate memory for time.");
	}
	else{
		time(time_val);
	}
	curl_easy_setopt(hCurl, CURLOPT_PRIVATE, time_val);

	// Perform request.
	CURLcode result = curl_easy_perform(hCurl);
	
	if(result != CURLE_OK){
		return -1;
	} 
	save_request_statistics(name);
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
	zlog_debug(loggers->scenario, "size * nmemb = %u.", result);
#endif
	if(userp == NULL){
#ifdef DEBUG
		zlog_debug(loggers->scenario, "User data pointer is NULL. Nothing to do.");
#endif
		return result;
	}
	if(result == 0){
#ifdef DEBUG
		zlog_debug(loggers->scenario, "Result = 0. Nothing to do");
#endif
		return 0;
	}
	size_t current_size = ((response_data_struct*)userp)->size;
	
	void ** response = &(((response_data_struct*)userp)->data);
	
	*response = realloc(*response, result + current_size);	
	
	if(response == NULL){
		zlog_error(loggers->scenario, "Memory allocation/reallocation failed.");
		return 0;
	}
	
	void * response_last_byte = *response;
	if(current_size > 1){		
		response_last_byte += current_size - 1;
	}
#ifdef DEBUG
	zlog_debug(loggers->scenario, "Response pointer value: %p.\tresponse_last_byte pointer value: %p.", response, response_last_byte);
#endif
	memcpy(response_last_byte, buffer, result);
	if(*response == NULL){
		zlog_error(loggers->scenario, "Failed to copy from [buffer] to [userp->data].");
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