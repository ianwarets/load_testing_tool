#include <time.h>
#include <zlog.h>

typedef enum{
    SUCCESS,
    FAIL
} transaction_status;

typedef struct{
    time_t start;
    time_t end;
    char * name;
    transaction_status status;
} transaction;

transaction transaction_begin(char *);
void transaction_end(transaction *, transaction_status);