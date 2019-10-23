#include "test_controller.h"
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

/**
 * Функция запуска/останова потоков ступени. 
 * Запускается в потоке, чтоб не задерживать отсчет времени для следующих ступеней.
 */
static void * control_step_threads(void * parameter){
    step_data * current_step = ((step_data*)parameter);
    action_data * actions = current_step->r_data->actions;
    unsigned int a_count = current_step->r_data->actions_count;
#ifdef DEBUG
    debug_message("%s %i threads with delay of %li s.", 
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
            // Stop threads.
            if(actions[i].running_threads >= thr_step_action){
                thr_for_cur_action = thr_step_action;
            }
            else{
                thr_for_cur_action = actions[i].running_threads;
            }
            
        }
                
        for(unsigned long t = 0; t < thr_for_cur_action; t++){            
            if(current_step->to_start){             
                actions[i].threads[t].stop_thread = 0;
                actions[i].threads[t].action = &actions[i];
                actions[i].threads[t].index = t + 1;
                int result = pthread_create(&(actions[i].threads[t].handle), NULL, actions[i].runner, (void*)&actions[i].threads[t]);
                if(result != 0){
                    static char * err_message_template = "Failed to create thread for step #%u, action #%lu, thread #%lu. %s";
                    static const size_t err_size = 100;
                    char err_text[err_size];
                    switch (result){
                        case EAGAIN:
                            sprintf(err_text, "%s", "Insufficient resources to create another thread");
                            break;
                        case EINVAL:
                            sprintf(err_text, "%s", "Invalid settings in attr.");
                            break;
                        case EPERM:
                            sprintf(err_text, "%s", "No permission to set the scheduling policy and parameters specified in attr");
                            break;
                        default:
                            break;
                    }
                    error_message(err_message_template, current_step->step_index, i, t, err_text);
                    continue;
                }                    
#ifdef DEBUG
                debug_message("Thread # %i for step # %u created.", t, current_step->step_index);
#endif
            }            
            else{
#ifdef DEBUG
                debug_message("Signaling action #%lu, thread # %lu to stop", i, t);
#endif
                actions[i].threads[t].stop_thread = 1;
            }           
#ifdef DEBUG
            debug_message("Sleep for %li", slope_interval);
#endif
        //TODO: Realize this delay by Timer, because it is more accurate then Sleeo()
            usleep(slope_interval * 1000000); 
        }
    }
    return 0;
}
/**
 * Функция выполняет запуск\останов потоков каждой ступени теста.
 * Функция вызывается по истечении времени таймера
 */
static void step_routine(void * p_runner_data){
    pthread_t thread;
    int result = pthread_create(&thread, NULL, control_step_threads, p_runner_data);
    if(result != 0){
        static char * err_message_template = "Failed to create thread for step launch. [%s].";
        static const size_t err_size = 100;
        char err_text[err_size];
        switch (result){
            case EAGAIN:
                sprintf(err_text, "%s", "Insufficient resources to create another thread");
                break;
            case EINVAL:
                sprintf(err_text, "%s", "Invalid settings in attr.");
                break;
            case EPERM:
                sprintf(err_text, "%s", "No permission to set the scheduling policy and parameters specified in attr");
                break;
            default:
                break;
        }
        error_message(err_message_template, err_text);
    }  
}

void * test_controller(void * p_runner_data){
    runner_data * r_data = ((runner_data*)p_runner_data);
    
    //TODO: In case when no start delay, run first step imediate without timer creation
    long long next_time_interval = r_data->start_delay;
    // multiplexer for convert seconds to 100ns intervals
    //long long multiplexer = -10000000LL;
    //LARGE_INTEGER start_time = {.QuadPart = next_time_interval * multiplexer};
#ifdef DEBUG
    debug_message("Start delay : %li", r_data->start_delay);    
#endif

    /* HANDLE step_timer = CreateWaitableTimer(&security_attr, TRUE, "Local: Step timer.");
    if(step_timer == NULL){
        error_message("Failed to create timer.");
        ExitThread(ERR_CREATE_TIMER);
        return 1;
    } */

    for(unsigned int step_index = 0; step_index < r_data->steps_count; step_index++){
/*#ifdef DEBUG
        debug_message("Timer value is: %lli.", start_time.QuadPart);
#endif*/
        /* BOOL result = SetWaitableTimer(step_timer, &start_time, 0, step_routine, p_runner_data, FALSE);
        if(!result){
            error_message("Failed to set timer for step # %u.",  step_index);
            ExitThread(ERR_SET_TIMER);
        } */
#ifdef DEBUG
        debug_message("Sleep for INFINITE.");
#endif
        if(usleep(next_time_interval * 1000000) != 0){
            error_message("Failed to make sleep");
        }
        step_routine(&r_data->steps[step_index]);

        /* info_message("Canceling waitable timer.");
        result = CancelWaitableTimer(step_timer);
        if(!result){
            error_message("Failed to cancel timer for step # %u.", step_index);
            ExitThread(ERR_CANCEL_TIMER);
        } */
        next_time_interval = r_data->steps[step_index].duration +  r_data->steps[step_index].slope_delay;
        //start_time.QuadPart = next_time_interval * multiplexer;
    }

   /*  if(!CloseHandle(step_timer)){
        error_message("Failed to close timer handler.");
    } */
    return 0;
}
