#include "action_wrappers.h"
#include "test_controller.h"
#include "ltt_common.h"
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/*
    Выполнение действия без интервала, следующая интерация запускается сразу.
*/
static void no_pacing_runner(void(*action)(), void * parameter, unsigned long pace_time){
    action(parameter);
}

/*
    Выполняет действие с интервалом, относительно начала выполнения действия.
*/
static void fixed_pacing_runner(void(*action)(), void * parameter, unsigned long pace_time_s){
    unsigned long long pace_time = pace_time_s * 1000000000;
    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);
    action(parameter);
    clock_gettime(CLOCK_MONOTONIC, &finish);
    unsigned long long diff = get_time_difference(start, finish) ;
    if(diff >= pace_time){
        return;
    }
    long long sleep_time = pace_time - diff;
    if(sleep_time){
        usleep(sleep_time / 1000);
    }
}
/*
    Выполняет действие с интервалом, относительно окончания выполнения действия.
*/
static void relative_pacing_runner(void(*action)(), void * parameter, unsigned long delay_time){    
    action(parameter);
    usleep(delay_time * 100000);
}

__thread _Atomic int e_signal_flag = 0;
static void sig_handler(int sig){
    e_signal_flag = 1;
}

/*
    Thread routine function
*/
static void* actions_wrapper(void * thread_params, void(*pacing_function)()){   
    thread_data * thread = (thread_data*)thread_params;
    if(signal(SIGSEGV, sig_handler) == SIG_ERR){
        printf("Failed to set signal handler for action %ls, thread # %i\n",thread->action->name , thread->index);
        pthread_exit((void*)EXIT_FAILURE);
    }
    
    action_data * action = thread->action;
    void (*init_routine)() = action->init;
    void (*action_routine)() = action->action;
    void (*end_routine)() = action->end;
    
    action->running_threads++;
    init_routine();
    do{
        if(e_signal_flag){
            error_message(L"Error occured in thread # %i\n", thread->index);
            break;
        }
        pacing_function(action_routine, thread_params, action->pacing);
    }
    while(!thread->stop_thread);    
    end_routine();
    action->running_threads--;
    pthread_exit((void*)EXIT_SUCCESS);
}

void * no_pacing(void * thread_params){
    return actions_wrapper(thread_params, no_pacing_runner);
}
void * fixed_pacing(void * thread_params){
    return actions_wrapper(thread_params, fixed_pacing_runner);
}
void * relative_pacing(void * thread_params){
    return actions_wrapper(thread_params, relative_pacing_runner);
}
