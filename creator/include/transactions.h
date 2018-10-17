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

void transaction_init();
transaction transaction_begin(char * name);
void transaction_end(transaction * trnsctn, transaction_status);