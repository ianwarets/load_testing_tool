typedef enum{
    SUCCESS,
    FAIL
} transaction_status;

typedef struct{   
    long long start_time_ms; 
    long long start_ticks_count;
    long long end_ticks_count;
    char * name;
    transaction_status status;
} transaction;

transaction transaction_begin(char *);
void transaction_end(transaction *, transaction_status);