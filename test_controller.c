#include "test_controller.h"
#include <windows.h>

extern zlog_categories * loggers;
/**
 * Функция запуска/останова потоков ступени. 
 * Запускается в потоке, чтоб не задерживать отсчет времени для следующих ступеней.
 */
static DWORD WINAPI control_step_threads(LPVOID parameter){
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);

    step_data * current_step = ((step_data*)parameter);
    action_data * actions = current_step->r_data->actions;
    unsigned int a_count = current_step->r_data->actions_count;
#ifdef DEBUG
    zlog_debug(loggers->common, 
            "%s %i threads with delay of %li s.", 
            current_step->to_start ? "Starting" : "Stopping",
            current_step->threads_count,
            current_step->slope_delay);
#endif
    long slope_interval = (current_step->slope_delay * 1000) / current_step->threads_count;
    unsigned long thr_rest = current_step->threads_count;    
    unsigned long step_thr_count = current_step->threads_count;

    if(current_step->to_start){
        unsigned long tot_run_thr = 0;
        unsigned long tot_stop_thr = 0;
        for(unsigned int i = 0; i <= current_step->step_index; i++){
            if(current_step->r_data->steps[i].to_start){
                tot_run_thr += current_step->r_data->steps[i].threads_count;
            }
            else{
                tot_stop_thr += current_step->r_data->steps[i].threads_count;
            }
        }
        step_thr_count = tot_run_thr - tot_stop_thr;               
    }

    for(unsigned long i = 0; i < a_count; i++){
        unsigned long thr_ratio = (actions[i].ratio * step_thr_count);
        unsigned long thr_step_action = thr_ratio / 100 + ((thr_ratio % 100) > 50 ? 1 : 0);
        if(thr_rest < thr_step_action){
            thr_step_action = thr_rest;
        }
        thr_rest -= thr_step_action;
        unsigned long thr_for_cur_action = 0;
        if(current_step->to_start){
            thr_for_cur_action = thr_step_action - actions[i].running_threads;     
        }   
        else{
            thr_for_cur_action = thr_step_action;
        }
                
        for(unsigned long t = 0; t < thr_for_cur_action; t++){            
            if(current_step->to_start){             
                actions[i].threads[t].stop_thread = 0;
                actions[i].threads[t].action = &actions[i];
                actions[i].threads[t].index = t + 1;
                actions[i].threads[t].handle = CreateThread(NULL, 0, actions[i].runner, (PVOID)&actions[i].threads[t], 0, NULL);
                if(actions[i].threads[t].handle == NULL){
                    zlog_error(loggers->common, "Failed to create thread for step #%u, action #%lu, thread #%lu.", current_step->step_index, i, t);
                    continue;
                }                    
#ifdef DEBUG
                zlog_debug(loggers->common, "Thread # %i for step created.", t);
#endif
            }            
            else{
#ifdef DEBUG
                zlog_debug(loggers->common, "Signaling action #%lu, thread # %lu to stop", i, t);
#endif
                actions[i].threads[t].stop_thread = 1;
            }           
#ifdef DEBUG
            zlog_debug(loggers->common, "Sleep for %li", slope_interval);
#endif
            Sleep(slope_interval); 
        }
    }
    return 0;
}
/**
 * Функция выполняет запуск\останов потоков каждой ступени теста.
 * Функция вызывается по истечении времени таймера
 */
static VOID CALLBACK step_routine(LPVOID p_runner_data, DWORD lowTimer, DWORD highTimer){
    HANDLE step_thread = CreateThread(NULL, 0, control_step_threads, p_runner_data, 0, NULL);
    if(!step_thread){
        zlog_error(loggers->common, "Failed to create thread for step launch. [%s].", GetLastError());        
    }  
}

DWORD WINAPI test_controller(LPVOID p_runner_data){
    runner_data * r_data = ((runner_data*)p_runner_data);
    /*struct _SECURITY_ATTRIBUTES security_attr;
    security_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_attr.bInheritHandle = FALSE;
    security_attr.lpSecurityDescriptor = NULL;
    */
    //TODO: In case when no start delay, run first step imediate without timer creation
    long long next_time_interval = r_data->start_delay;
    // multiplexer for convert seconds to 100ns intervals
    //long long multiplexer = -10000000LL;
    //LARGE_INTEGER start_time = {.QuadPart = next_time_interval * multiplexer};
#ifdef DEBUG
    zlog_debug(loggers->common, "Start delay : %li", r_data->start_delay);    
#endif

    /* HANDLE step_timer = CreateWaitableTimer(&security_attr, TRUE, "Local: Step timer.");
    if(step_timer == NULL){
        zlog_error(loggers->common, "Failed to create timer.");
        ExitThread(ERR_CREATE_TIMER);
        return 1;
    } */

    for(unsigned int step_index = 0; step_index < r_data->steps_count; step_index++){
/*#ifdef DEBUG
        zlog_debug(loggers->common, "Timer value is: %lli.", start_time.QuadPart);
#endif*/
        /* BOOL result = SetWaitableTimer(step_timer, &start_time, 0, step_routine, p_runner_data, FALSE);
        if(!result){
            zlog_error(loggers->common, "Failed to set timer for step # %u.", step_index);
            ExitThread(ERR_SET_TIMER);
        } */
#ifdef DEBUG
        zlog_debug(loggers->common, "Sleep for INFINITE.");
#endif
        SleepEx(next_time_interval * 1000, TRUE);
        step_routine(&r_data->steps[step_index], 0, 0);

        /* zlog_info(loggers->common, "Canceling waitable timer.");
        result = CancelWaitableTimer(step_timer);
        if(!result){
            zlog_error(loggers->common, "Failed to cancel timer for step # %u.", step_index);
            ExitThread(ERR_CANCEL_TIMER);
        } */
        next_time_interval = r_data->steps[step_index].duration +  r_data->steps[step_index].slope_delay;
        //start_time.QuadPart = next_time_interval * multiplexer;
    }

   /*  if(!CloseHandle(step_timer)){
        zlog_error(loggers->common, "Failed to close timer handler.");
    } */
    return 0;
}
