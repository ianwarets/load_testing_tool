#include <windows.h>
#include "transactions.h"
#include <logger.h>


static void save_statistics(transaction *);
static long long get_duration(transaction *);
static long long get_ticks_count();
static char * convert_systime_to_date_time(SYSTEMTIME st);
static zlog_categories * loggers;

void transaction_init(){
    loggers = logger_get_loggers();
}

transaction transaction_begin(char * name){
    // Получить текущее значение времени. Передать полученное значение и название транзакции в хранилище    
    SYSTEMTIME lt;
    GetSystemTime(&lt);
    
    long long ticks = get_ticks_count();
    transaction tr = {        
        .name = name,
        .start_ticks_count = ticks,
        .start_time = convert_systime_to_date_time(lt)
    };   

    zlog_info(loggers->common
            , "Transaction:\"%s\" - Start time: %s"
            , name, tr.start_time);
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
    free(transaction->start_time);    
}

static void save_statistics(transaction * transaction){
    if(transaction == NULL){
        zlog_error(loggers->common, "Transaction is NULL");
        return;
    }
    char * status = transaction->status == SUCCESS ? "SUCCESS" : "FAIL";
    long long duration = get_duration(transaction);
#ifdef DEBUG
    zlog_debug(loggers->common
                , "Start time: %s. Duration: %lli. Name: %s. Status: %s"
                , transaction->start_time, duration, transaction->name, status);
#endif
    char * format_string = "%s,%s,%llu,%s";
    
    zlog_info(loggers->transactions,
                format_string,
                transaction->start_time,
                transaction->name,
                duration,
                status);
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
    difference /= frequency.QuadPart;
    return difference;
}

static long long get_ticks_count(){
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return time.QuadPart;
}

static char * convert_systime_to_date_time(SYSTEMTIME st){
    char * const date_time_format = "%02d.%02d.%d %02d:%02d:%02d.%03d";
    char * output = (char *)malloc(strlen(date_time_format) + 1);
    if(output == NULL){
        zlog_error(loggers->common, "Failed to allocate memory for date_time convertion output.");
    }
    sprintf(output, date_time_format, st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return output;
}