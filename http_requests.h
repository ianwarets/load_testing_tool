/*
    Структура, для хранения полученных данных и их объема
*/
typedef struct {
	void * data;
	size_t size;
}response_data_struct;

typedef struct {
    void * response_time;
    void * netw_delay;
    long uploaded_bytes;
    long received_bytes;
    long upload_speed;
    long download_speed;    
}transaction_tech_data;

CURLcode get_request(CURL *, char*, struct curl_slist *, response_data_struct *);
CURLcode post_request(CURL *, char*, struct curl_slist *, response_data_struct *);
