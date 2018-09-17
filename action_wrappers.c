#include "action_wrappers.h"
#include "test_controller.h"

void no_pacing(void(*action)(), void * parameter, DWORD pace_time){
    action(parameter);
}

void fixed_pacing(void(*action)(), void * parameter, DWORD pace_time){
    ULONGLONG start = GetTickCount64();
    action(parameter);
    ULONGLONG finish = GetTickCount64();
    ULONGLONG diff = finish - start;
    DWORD sleep_time = pace_time - diff;
    if(sleep_time){
        SleepEx(pace_time, TRUE);
    }
    return;
}

void delayed_pacing(void(*action)(), void * parameter, DWORD delay_time){
    thread_data * t_data = (thread_data*)parameter;
    while(!t_data->stop_flag){
        action(t_data->parameter);
        SleepEx(delay_time, TRUE);
    }
}

void run_action(void(*action)(), void * parameter, void(*pacing_function)(), DWORD delay_time){
    thread_data * t_data = (thread_data*)parameter;
    while(!t_data->stop_flag){
        pacing_function(action, parameter, delay_time);
    }
}
