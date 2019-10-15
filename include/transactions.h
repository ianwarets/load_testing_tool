#include <time.h>

typedef enum{
    SUCCESS,
    FAIL
} transaction_status;

typedef struct{
    struct timespec start_time;
    struct timespec end_time;
    char * name;
    transaction_status status;
} transaction;

void transaction_init();
transaction transaction_begin(char * name);
void transaction_end(transaction * trnsctn, transaction_status);