#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <zlog.h>

typedef struct{
    void(*init)();
    void(*end)();
    void(*action)();
    void * user_data;
    zlog_category_t * statistics;
    zlog_category_t * thread_log;
    zlog_category_t * programm_log;
} action_data;

typedef struct{
    HANDLE thread;
    _Atomic int stop_flag;
    action_data * parameter;
    LPTHREAD_START_ROUTINE run_action_function;
    DWORD pacing;
} thread_data;

typedef struct step_data step_data;
struct step_data{
    long next_step_time_interval;
    int threads_count;
    thread_data * threads_array;    
    void * param;
    BOOL to_start; //run or stop threads
    long slope_delay; // used for slope providing
};

typedef struct {
    step_data * steps;
    long steps_count;
    long start_delay;
} runner_data;

enum thread_errors {
    ERR_CREATE_TIMER = 1,
    ERR_SET_TIMER,
    ERR_CANCEL_TIMER
};