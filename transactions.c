#include "transactions.h"
#include "logger.h"
#include <time.h>

static void save_statistics(transaction * transaction);
extern zlog_categories * loggers;

transaction transaction_begin(char * name){
    // Получить текущее значение времени. Передать полученное значение и название транзакции в хранилище
    
    return (transaction){
        .start = time(NULL),
        .name = name
    };
}

void transaction_end(transaction * transaction, transaction_status status){
    time_t end = clock();
    if(transaction == NULL){
        zlog_error(loggers->common, "Transaction is NULL");
        return;
    }
    transaction->end = time(NULL);
    transaction->status = status;
    save_statistics(transaction);
}

static void save_statistics(transaction * transaction){
    if(transaction == NULL){
        zlog_error(loggers->common, "Transaction is NULL");
        return;
    }
    char * status = transaction->status ? "SUCCESS" : "FAIL";
    zlog_debug(loggers->common
                , "Start time: %li. End time: %li. Name: %s. Status: %s"
                , transaction->start, transaction->end, transaction->name, status);
    char * format_string = "%li,%s,%li,%s";
    long duration = transaction->end - transaction->start;
    zlog_info(loggers->statistics, format_string, transaction->start, transaction->name, duration, status);
    return;
}