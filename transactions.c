#include "transactions.h"
#include "ltt_common.h"
#include <logger.h>
#include <stdio.h>
#include <stdlib.h>


static void save_statistics(transaction *);
static unsigned long long get_duration(transaction *);

transaction transaction_begin(char * name){
    // Получить текущее значение времени. Передать полученное значение и название транзакции в хранилище    
    struct timespec lt;
    clock_gettime(CLOCK_MONOTONIC, &lt);
    
    transaction tr = {        
        .name = name,
        .start_time = lt,
    };   

    info_message("Transaction:\"%s\" - Start time: %li", name, tr.start_time.tv_sec);
    return tr;
}

void transaction_end(transaction * transaction, transaction_status status){
    if(transaction == NULL){
        error_message("Transaction is NULL");
        return;
    }
    struct timespec lt;
    clock_gettime(CLOCK_MONOTONIC, &lt);

    transaction->end_time = lt;
    transaction->status = status;
    save_statistics(transaction);   
}

static void save_statistics(transaction * transaction){
    if(transaction == NULL){
        error_message("Transaction is NULL");
        return;
    }
    char * status = transaction->status == SUCCESS ? "SUCCESS" : "FAIL";
    unsigned long long duration = get_duration(transaction);
#ifdef DEBUG
    debug_message("Start time: %li. Duration: %lli. Name: %s. Status: %s"
                , transaction->start_time.tv_sec, duration, transaction->name, status);
#endif
    info_message("%li,%s,%llu,%s",
                transaction->start_time.tv_sec,
                transaction->name,
                duration,
                status);
    return;
}

/**
 * Returns the operation duration in nanoseconds * 
 */
static unsigned long long get_duration(transaction * trnsctn){
    return get_time_difference(trnsctn->start_time, trnsctn->end_time);
}