#include "logger.h"
#include "action_wrappers.h"
#include <stdio.h>
#include <stdint.h>
#include <wchar.h>
#include <pthread.h>

//#pragma once
enum thread_errors {
    ERR_CREATE_TIMER = 1,
    ERR_SET_TIMER,
    ERR_CANCEL_TIMER
};

/// @brief Struct for storing action information. Threads, functions to execute, ref to test case library, pacing and ratio
typedef struct action action_data;

/// @brief Struct for storing execution thread information.
typedef struct thread{
	_Atomic int stop_thread;	
	unsigned int index;
	// Thread handle
	pthread_t handle;
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
	// Pointer to dynamicly loaded library.
	void * action_lib_handler;
	// Pointer to runner function.
	p_pacing_function runner;
    unsigned long pacing;
    unsigned int ratio;
};

/// @brief Struct for storing tesp plan data. Steps, threads count, actions/
typedef struct runner runner_data;

typedef struct step{
	unsigned int step_index;
	// Indicate step type: run or stop  threads.
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
void * test_controller(void *);

