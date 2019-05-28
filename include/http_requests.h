#include <curl/curl.h>
#include "common_types.h"

/**
 *   Структура, для хранения полученных данных и их объема
 */
typedef struct {
	void * data;
	size_t size;
}response_data_struct;

/**
 * Получение объекта для работы с запросами.
 */
EXPORT CURL * http_request_get_handler();

/**
 *	Выполнение GET запроса.
 *	@param name - имя запроса.
 *	@param hCurl - объект библиотеки CURL.
 * 	@param url - адрес выполнения запроса.
 *	@param headers - указатель на структуру хранящую заголовки запроса. struct curl_slist.
 * 	@param response_data - указатель на структуру предназначенную для сохранения полученного в результате запроса ответа. 
 *	@return код ответа
 */
EXPORT  long get_request(char * name, char * url, struct curl_slist * headers, response_data_struct * response_data);

/**
 *	Выполнение POST запроса.
 *	@param name - имя запроса.
 * 	@param url - адрес выполнения запроса.
 *	@param headers - указатель на структуру хранящую заголовки запроса. struct curl_slist.
 * 	@param response_data - указатель на структуру предназначенную для сохранения полученного в результате запроса ответа. 
 *	@return код ответа
 */
EXPORT long post_request(char * name, char * url, struct curl_slist * headers, response_data_struct * response_data, char * post_data, long post_data_size);

/**
 *	Получить значение заголовка Location 
 */
EXPORT char * get_redirect_link();

