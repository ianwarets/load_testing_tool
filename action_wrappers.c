#include "action_wrappers.h"
#include "test_controller.h"

/*
    Выполнение действия без интервала, следующая интерация запускается сразу.
*/
void no_pacing(void(*action)(), void * parameter, DWORD pace_time){
    action(parameter);
}

/*
    Выполняет действие с интервалом, относительно начала выполнения действия.
*/
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
/*
    Выполняет действие с интервалом, относительно окончания выполнения действия.
*/
void delayed_pacing(void(*action)(), void * parameter, DWORD delay_time){    
    action(parameter);
    SleepEx(delay_time, TRUE);
}

DWORD WINAPI actions_wrapper(LPVOID thread_params){
    thread_data * t_data = (thread_data*)thread_params;
    void (*init_routine)() = t_data->parameter->init;
    void (*action)() = t_data->parameter->action;
    void (*end_routine)() = t_data->parameter->end;

    init_routine(t_data->parameter->user_data);
    do{
        pacing_function(action, thread_params, t_data->pacing);
    }
    while(!t_data->stop_flag);    
    end_routine(t_data->parameter);
}
