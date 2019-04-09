#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <wchar.h>
#include "logger.h"

//#pragma once
enum thread_errors {
    ERR_CREATE_TIMER = 1,
    ERR_SET_TIMER,
    ERR_CANCEL_TIMER
};

typedef struct action action_data;

typedef struct thread{
	_Atomic int stop_thread;	
	unsigned int index;
	void * handle;
	action_data * action;
} thread_data;

struct action{
	thread_data * threads;
	unsigned int threads_count;
	_Atomic unsigned int running_threads;
	void(*init)();
	void(*action)();
	void(*end)();
	wchar_t * name;
	HMODULE action_lib_handler;
	LPTHREAD_START_ROUTINE runner;
    unsigned long pacing;
    unsigned int ratio;
};

typedef struct runner runner_data;

typedef struct step{
	unsigned int step_index;
	//indicate step type: run or stop  threads
	int to_start;
    unsigned long duration;
	unsigned long slope_delay;
    unsigned long threads_count;
	runner_data * r_data;
} step_data;

struct runner{
	unsigned int steps_count;
	step_data * steps;
    unsigned long start_delay;	
    unsigned long total_threads_count;
    action_data * actions;
	unsigned int actions_count;
};

/**
 *  Запуск ступеней теста по таймеру. 
 */
DWORD WINAPI test_controller(LPVOID);

