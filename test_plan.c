#include <stdio.h>
#include "test_plan.h"
#include <ujdecode.h>
#include <zlog.h>

static int create_test_plan_from_json(char *, test_plan *);

char * read_test_plan(char * test_plan_name){
    return NULL;
}

runner_data * generate_runner_data(char * test_plan){
    long steps_count = 0;
    long start_delay = 0;
    runner_data * run_data = (runner_data*)malloc(sizeof(runner_data));
    run_data->steps = (step_data*)malloc(sizeof(step_data) * steps_count);
    run_data->start_delay = start_delay;
    run_data->steps_count = steps_count;
    return NULL;
}

void free_runner_data(runner_data * run_data, void(*free_user_data)()){    
    for(long i = 0; i < run_data->steps_count; i++){
        step_data s_data = run_data->steps[i];
        thread_data * threads_array = s_data.threads_array;
        for(long n = 0; n < s_data.threads_count; n++){
            free_user_data(threads_array[n].parameter);
        }
        free(threads_array);
    }
    free(run_data->steps);
}

static int create_test_plan_from_json(char * json_string, test_plan * plan){
    void * state;
    test_plan t_plan = *plan;
    int objects_count;
    UJObject obj =  UJDecode(json_string, strlen(json_string), NULL, &state);
    if(obj == NULL){
        //log error
        //zlog_error(stderr, "Failed to parse test plan file.\n%s", UJGetError(state));
        return 1;
    }
    const wchar_t * tree_keys[] = { L"test_name", L"actions", L"steps"};
    UJObject o_test_name, o_actions, o_steps;
    objects_count = 3;
    if(UJObjectUnpack(obj, objects_count, "SAA", tree_keys, &o_test_name, &o_actions, &o_steps) != objects_count){
        //log error
        //zlog_error(stderr, "Failed to unpack JSON object");
        UJFree(state);
        return 1;
    }
    t_plan.name = UJReadString(o_test_name, NULL);
    void * iterator = UJBeginArray(o_actions);
    if(iterator == NULL){
        //log error
        //zlog_error(stderr, "Failed to create JSON array iterator.");
        UJFree(state);
        return 1;
    }
    UJObject o_action, o_act_file_name, o_pacing, o_percentage;
    const wchar_t * action_keys[] = { L"name", L"pacing", L"percentage" };
    objects_count = 3;
    int i = 1;    
    while(UJIterArray(&iterator, o_action)){
        t_plan.actions_count = i;
        t_plan.actions = (action*)realloc(t_plan.actions, sizeof(action) * i);
        if(t_plan.actions == NULL){
            //log error
            //zlog_error(stderr, "Actions realloc failed");
            UJFree(state);
            return 1;
        }  
        if(UJObjectUnpack(o_action, objects_count, "SNN", action_keys, o_act_file_name, o_pacing, o_percentage) != objects_count){
            //log error
            //zlog_error(stderr, "Failed to unpack JSON action object");
            return 1;            
        }
        t_plan.actions[i].action_file_name = UJReadString(o_act_file_name, NULL);
        if(t_plan.actions[i].action_file_name == NULL){
            //zlog_error(stderr, "Failed to get action file name");
            UJFree(state);
            return 1;
        }
        t_plan.actions[i].pacing = UJNumericInt(o_pacing);
        t_plan.actions[i].percentage = UJNumericInt(o_percentage);     
        i++;
    }

    iterator = UJBeginArray(o_steps);
    if(iterator == NULL){
        //zlog_error(stderr, "Failed to get steps iterator");
        UJFree(state);
        return 1;
    }

    UJObject o_step, o_step_act_type, o_step_run_duration, o_step_threads_count, o_step_slope_duration;
    objects_count = 4;
    i = 0;
    const wchar_t step_keys = { L"name", L"action_type", L"run_duration", L"threads_count", L"slope_duration"};
    while(UJIterArray(iterator, o_step)){
        t_plan.steps_count = i;
        t_plan.steps = (test_step*)realloc(t_plan.steps, sizeof(test_step) * i);
        if(t_plan.steps == NULL){
            //zlog_error(stderr, "Steps realloc failed");
            UJFree(state);
            return 1;
        }
        if(UJObjectUnpack(o_steps, objects_count, "SNNN", step_keys, o_step_act_type, o_step_run_duration, o_step_threads_count, o_step_slope_duration) != objects_count){
            //zlog_error(stderr, "Failed to unpack JSON step object");
            UJFree(state);
            return 1;
        }
        size_t str_size = 0;
        wchar_t * act_type = UJReadString(o_step_act_type, &str_size);
        t_plan.steps[i].action_type = (wchar_t *)malloc(sizeof(wchar_t) * (str_size + 1));
        if(t_plan.steps[i].action_type == NULL){
            //zlog_error(stderr, "Action_type malloc failed");
            return 1;
        }
        wcscpy(t_plan.steps[i].action_type, act_type);
        t_plan.steps[i].run_duration = UJNumericInt(o_step_run_duration);
        t_plan.steps[i].threads_count = UJNumericInt(o_step_threads_count);
        t_plan.steps[i].slope_duration = UJNumericInt(o_step_slope_duration);
        i++;
    }
    UJFree(state);
    return 0;
}