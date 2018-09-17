#include <windows.h>
#include <stdio.h>
#include <stdint.h>

typedef struct{
    HANDLE thread;
    _Atomic int stop_flag;
    void * parameter;
} thread_data;

typedef struct step_data step_data;
struct step_data{
    long next_step_time_interval;
    int threads_count;
    thread_data * threads_array;
    LPTHREAD_START_ROUTINE action;
    void * param;
    BOOL to_start; //run or stop threads
    long slope_delay; // used for slope providing
    step_data * next_step;
};

typedef struct {
    step_data * steps;
    long steps_count;
    long current_step_index;
    long start_delay;
} runner_data;

enum thread_errors {
    ERR_CREATE_TIMER = 1,
    ERR_SET_TIMER,
    ERR_CANCEL_TIMER
};