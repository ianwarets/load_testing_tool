#include "action_wrappers.h"
#include "test_controller.h"

/*
    Выполнение действия без интервала, следующая интерация запускается сразу.
*/
static void no_pacing_runner(void(*action)(), void * parameter, DWORD pace_time){
    action(parameter);
}

/*
    Выполняет действие с интервалом, относительно начала выполнения действия.
*/
static void fixed_pacing_runner(void(*action)(), void * parameter, DWORD pace_time){
    ULONGLONG start = GetTickCount64();
    action(parameter);
    ULONGLONG finish = GetTickCount64();
    ULONGLONG diff = finish - start;
    DWORD sleep_time = pace_time - diff;
    if(sleep_time){
        SleepEx(pace_time, TRUE);
    }
}
/*
    Выполняет действие с интервалом, относительно окончания выполнения действия.
*/
static void relative_pacing_runner(void(*action)(), void * parameter, DWORD delay_time){    
    action(parameter);
    SleepEx(delay_time, TRUE);
}

/*
    Thread routine function
*/
static void actions_wrapper(LPVOID thread_params, void(*pacing_function)()){
    thread_data * t_data = (thread_data*)thread_params;
    void (*init_routine)() = t_data->action->init;
    void (*action)() = t_data->action->action;
    void (*end_routine)() = t_data->action->end;

    init_routine(t_data->action->user_data);
    do{
        pacing_function(action, thread_params, t_data->pacing);
    }
    while(!t_data->stop_flag);    
    end_routine(t_data->action);
}

DWORD WINAPI no_pacing(LPVOID thread_params){
    actions_wrapper(thread_params, no_pacing_runner);
}
DWORD WINAPI fixed_pacing(LPVOID thread_params){
    action_wrapper(thread_params, fixed_pacing_runner);
}
DWORD WINAPI relative_pacing(LPVOID thread_params){
    actions_wrapper(thread_params, relative_pacing_runner);
}
