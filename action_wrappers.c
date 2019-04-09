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
    //ULONGLONG start = GetTickCount64();
    pace_time *= 1000;
    DWORD start = GetTickCount();
    action(parameter);
    DWORD finish = GetTickCount();
    //ULONGLONG finish = GetTickCount64();
    //ULONGLONG diff = finish - start;
    DWORD diff = finish - start;
    if(diff >= pace_time){
        return;
    }
    DWORD sleep_time = pace_time - diff;
    if(sleep_time){
        SleepEx(sleep_time, TRUE);
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
static DWORD actions_wrapper(LPVOID thread_params, void(*pacing_function)()){    
    thread_data * thread = (thread_data*)thread_params;
    action_data * action = thread->action;
    void (*init_routine)() = action->init;
    void (*action_routine)() = action->action;
    void (*end_routine)() = action->end;
    
    action->running_threads++;
    init_routine();
    do{
        pacing_function(action_routine, thread_params, action->pacing);        
    }
    while(!thread->stop_thread);    
    end_routine();
    action->running_threads--;
    return 0;
}

DWORD WINAPI no_pacing(LPVOID thread_params){
    return actions_wrapper(thread_params, no_pacing_runner);
}
DWORD WINAPI fixed_pacing(LPVOID thread_params){
    return actions_wrapper(thread_params, fixed_pacing_runner);
}
DWORD WINAPI relative_pacing(LPVOID thread_params){
    return actions_wrapper(thread_params, relative_pacing_runner);
}
