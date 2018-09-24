#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "logger.h"

//#pragma once
enum thread_errors {
    ERR_CREATE_TIMER = 1,
    ERR_SET_TIMER,
    ERR_CANCEL_TIMER
};

typedef struct{
    void(*init)();
    void(*end)();
    void(*action)();
    void * user_data;
    HMODULE library_handler;
} action_data;

typedef struct{
    HANDLE thread;
    _Atomic int stop_flag;
    action_data * action;
    LPTHREAD_START_ROUTINE run_action_function;
    DWORD pacing;
} thread_data;

typedef struct {
    long next_step_time_interval;
    int threads_count;
    thread_data * threads_array;    
    void * param;
    BOOL to_start; //run or stop threads
    long slope_delay; // used for slope providing
}step_data;

typedef struct {
    step_data * steps;
    long steps_count;
    long start_delay;
    zlog_categories * loggers;
} runner_data;

/**
 *  Запуск ступеней теста по таймеру. 
 */
DWORD WINAPI test_controller(LPVOID);

