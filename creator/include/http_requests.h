/**
 *   Структура, для хранения полученных данных и их объема
 */
typedef struct {
	void * data;
	size_t size;
}response_data_struct;

typedef struct {
    //Request start time
    time_t * time;
    // Response time in microseconds.
    curl_off_t    response_time;
    // Network delay time in microseconds.
    curl_off_t    netw_delay;
    // Amount of uploaded bytes.
    curl_off_t    uploaded_bytes;
    // Amount of received bytes.
    curl_off_t    received_bytes;
    // Upload speed in bytes/second
    curl_off_t    upload_speed;
    // Download speed in bytes/second
    curl_off_t    download_speed;
}request_tech_data;

typedef struct {
    char * name;
    time_t time;
} request_private_data;

int init_http_requests();
void http_request_cleanup();
long get_request(char *, char*, struct curl_slist *, response_data_struct *);
long post_request(char *, char*, struct curl_slist *, response_data_struct *);
char * get_redirect_link();
