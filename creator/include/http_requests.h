#include <curl/curl.h>
/**
 *   Структура, для хранения полученных данных и их объема
 */
typedef struct {
	void * data;
	size_t size;
}response_data_struct;

/**
 * Инициализация библиотеки запросов.
 * @return - 0 в случае успеха, 1 в случае неудачной инициализации.
 */
int init_http_requests();
void http_request_cleanup();


/**
 *	Выполнение GET запроса.
 *	@param name - имя запроса.
 *	@param hCurl - объект библиотеки CURL.
 * 	@param url - адрес выполнения запроса.
 *	@param headers - указатель на структуру хранящую заголовки запроса. struct curl_slist.
 * 	@param response_data - указатель на структуру предназначенную для сохранения полученного в результате запроса ответа. 
 *	@return код ответа
 */
long get_request(char * name, char * url, struct curl_slist * headers, response_data_struct * response_data);


/**
 *	Выполнение POST запроса.
 *	@param name - имя запроса.
 * 	@param url - адрес выполнения запроса.
 *	@param headers - указатель на структуру хранящую заголовки запроса. struct curl_slist.
 * 	@param response_data - указатель на структуру предназначенную для сохранения полученного в результате запроса ответа. 
 *	@return код ответа
 */
long post_request(char * name, char * url, struct curl_slist * headers, response_data_struct * response_data);


/**
 *	Получить значение заголовка Location 
 */
char * get_redirect_link();
