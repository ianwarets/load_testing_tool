#include "test_controller.h"

zlog_categories * loggers;
/**
 * Функция запуска/останова потоков ступени. 
 * Запускается в потоке, чтоб не задерживать отсчет времени для следующих ступеней.
 */
static DWORD WINAPI run_step_threads(LPVOID parameter){
    step_data current_step = *((step_data*)parameter);
    zlog_debug(loggers->common, 
            "%s %i threads with delay of %li", 
            current_step.to_start ? "Starting" : "Stopping",
            current_step.threads_count,
            current_step.slope_delay);
    long slope_interval = current_step.slope_delay / current_step.threads_count;
    if(current_step.to_start){        
        for(long i = 0; i < current_step.threads_count; i++){
            current_step.threads_array[i].thread = CreateThread(NULL, 0, current_step.threads_array[i].run_action_function, current_step.param, 0, NULL);
            Sleep(current_step.slope_delay);
        }
    }
    else{
        for(long i = 0; i < current_step.threads_count; i++){
            current_step.threads_array[i].stop_flag = 1;
            Sleep(current_step.slope_delay);                       
        }
    }
    return 0;
}
/**
 * Функция выполняет запуск\останов потоков каждой ступени теста.
 * Функция вызывается по истечении времени таймера
 */
static VOID CALLBACK step_routine(LPVOID p_step_data, DWORD lowTimer, DWORD highTimer){
    zlog_info(loggers->common, "Running step routine thread.");
    HANDLE step_thread = CreateThread(NULL, 0, run_step_threads, p_step_data, 0, NULL);
    if(!step_thread){
        zlog_error(loggers->common, "Failed to create thread for step launch.");        
    }  
}

DWORD WINAPI test_controller(LPVOID p_runner_data){
    runner_data r_data = *((runner_data*)p_runner_data);
    loggers = r_data.loggers;
    struct _SECURITY_ATTRIBUTES security_attr;
    security_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_attr.bInheritHandle = FALSE;
    security_attr.lpSecurityDescriptor = NULL;
    
    LARGE_INTEGER start_time = {.QuadPart = 0};
    long duration = r_data.start_delay;
    zlog_debug(loggers->common, "Start delay : %li", duration);
    step_data * step;    
    long step_index = 0;
    while(step_index < r_data.steps_count){
        step = &(r_data.steps[step_index++]); 
        HANDLE step_timer = CreateWaitableTimer(&security_attr, TRUE, "Local: Step timer.");
        if(step_timer == NULL){
            zlog_error(loggers->common, "Failed to create timer for step № %i", step_index);
            ExitThread(ERR_CREATE_TIMER);
        }
        BOOL result = SetWaitableTimer(step_timer, &start_time, duration, step_routine, step, FALSE);
        if(!result){
            zlog_error(loggers->common, "Failed to set timer for step № %i", step_index);
            ExitThread(ERR_SET_TIMER);
        }
        zlog_debug(loggers->common, "Thread sleep INFINITE.");
        SleepEx(INFINITE, TRUE);
        result = CancelWaitableTimer(step_timer);
        if(!result){
            zlog_error(loggers->common, "Failed to cancel timer for step № %i", step_index);
            ExitThread(ERR_CANCEL_TIMER);
        }
        duration = step->next_step_time_interval + step->slope_delay;        
    }
    return 0;
}
