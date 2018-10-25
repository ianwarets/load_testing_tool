typedef enum{
    SUCCESS,
    FAIL
} transaction_status;

typedef struct{   
    char * start_time; 
    unsigned long long start_ticks_count;
    unsigned long long end_ticks_count;
    char * name;
    transaction_status status;
} transaction;

void transaction_init();
transaction transaction_begin(char * name);
void transaction_end(transaction * trnsctn, transaction_status);