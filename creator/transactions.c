#include "transactions.h"
#include <logger.h>
#include <windows.h>

static void save_statistics(transaction *);
static long long get_duration(transaction *);
static long long get_ticks_count();
extern zlog_categories * loggers;

transaction transaction_begin(char * name){
    // Получить текущее значение времени. Передать полученное значение и название транзакции в хранилище    
    transaction tr = {        
        .name = name,
        .start_ticks_count = get_ticks_count()
    };
    return tr;
}

void transaction_end(transaction * transaction, transaction_status status){
    if(transaction == NULL){
        zlog_error(loggers->common, "Transaction is NULL");
        return;
    }
    transaction->end_ticks_count = get_ticks_count();
    transaction->status = status;
    save_statistics(transaction);
}

static void save_statistics(transaction * transaction){
    if(transaction == NULL){
        zlog_error(loggers->common, "Transaction is NULL");
        return;
    }
    char * status = transaction->status ? "SUCCESS" : "FAIL";
    long long duration = get_duration(transaction);
    zlog_debug(loggers->common
                , "Start time: %lli. Duration: %lli. Name: %s. Status: %s"
                , transaction->start_time_ms, duration, transaction->name, status);
    char * format_string = "%li,%s,%lli,%s";
    zlog_info(loggers->statistics, format_string, transaction->start_time_ms, transaction->name, duration, status);
    return;
}

/**
 * Returns the operation duration in milliseconds
 * 
 */
static long long get_duration(transaction * trnsctn){
    static LARGE_INTEGER frequency; 
    if(frequency.QuadPart == 0){
        QueryPerformanceFrequency(&frequency);
    }
    long long difference = trnsctn->end_ticks_count - trnsctn->start_ticks_count;
    //To eliminate loss of precision convert result to microseconds.
    difference *= 1000000;
    difference /=frequency.QuadPart;
    return difference;
}

static long long get_ticks_count(){
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return time.QuadPart;
}