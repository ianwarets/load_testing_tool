#include "test_controller.h"

/**
 * Функция запуска/останова потоков ступени. 
 * Запускается в потоке, чтоб не задерживать отсчет времени для следующих ступеней.
 */
DWORD WINAPI run_step_threads(LPVOID parameter){
    step_data current_step = *((step_data*)parameter);
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
VOID CALLBACK step_routine(LPVOID p_step_data, DWORD lowTimer, DWORD highTimer){
    HANDLE step_thread = CreateThread(NULL, 0, run_step_threads, p_step_data, 0, NULL);
    if(!step_thread){
        //log error        
    }
    step_data current_step = *((step_data*)p_step_data);    
}

/**
 *  Запуск ступеней теста по таймеру. 
 */
DWORD WINAPI test_controller(LPVOID p_runner_data){
    runner_data r_data = *((runner_data*)p_runner_data);

    struct _SECURITY_ATTRIBUTES security_attr;
    security_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_attr.bInheritHandle = FALSE;
    security_attr.lpSecurityDescriptor = NULL;

    
    LARGE_INTEGER start_time;
    start_time.QuadPart = 0;
    long duration = r_data.start_delay;
    step_data * next_step = r_data.steps;    
    long step_index = 0;
    while(step_index < r_data.steps_count){
        next_step = &(r_data.steps[step_index++]); 
        HANDLE step_timer = CreateWaitableTimer(&security_attr, TRUE, "Local Steps timer.");
        if(step_timer == NULL){
            //log error
            ExitThread(ERR_CREATE_TIMER);
        }
        BOOL result = SetWaitableTimer(step_timer, &start_time, duration, step_routine, next_step, FALSE);
        if(!result){
            //log error
            ExitThread(ERR_SET_TIMER);
        }
        SleepEx(INFINITE, TRUE);
        result = CancelWaitableTimer(step_timer);
        if(!result){
            //log error
            ExitThread(ERR_CANCEL_TIMER);
        }

        duration = next_step->next_step_time_interval;        
    }
    return 0;
}
