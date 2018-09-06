/*
    Структура, для хранения полученных данных и их объема
*/
typedef struct {
	void * data;
	size_t size;
}response_data_struct;

typedef struct {
    // Response time in microseconds.
    curl_off_t  response_time;
    // Network delay time in microseconds.
    curl_off_t netw_delay;
    // Amount of uploaded bytes.
    curl_off_t uploaded_bytes;
    // Amount of received bytes.
    curl_off_t received_bytes;
    // Upload speed in bytes/second
    curl_off_t  upload_speed;
    // Download speed in bytes/second
    curl_off_t  download_speed;    
}transaction_tech_data;


CURLcode get_request(CURL *, char*, struct curl_slist *, response_data_struct *);
CURLcode post_request(CURL *, char*, struct curl_slist *, response_data_struct *);
transaction_tech_data collect_request_statistics(CURL *);
