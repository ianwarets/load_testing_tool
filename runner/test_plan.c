#include "test_plan.h"
#include <ujdecode.h>
#include "logger.h"
#include "action_wrappers.h"

extern zlog_categories * loggers;

static int create_test_plan_from_json(char *, test_plan *);
static LPTHREAD_START_ROUTINE define_pacing_function(const wchar_t *);
static int get_function_references(action_data *, const wchar_t *);

static char * read_test_plan(char * test_plan_name){
    zlog_info(loggers->common, "Opening test plan data file");
    FILE * f_test_plan = fopen(test_plan_name, "r");
    if(f_test_plan == NULL){
        zlog_error(loggers->common, "Failed to open test plan file");
        return NULL;
    }
    if(fseek(f_test_plan, 0, SEEK_END)){        
        zlog_error(loggers->common, "fseek failed to move to the end of file.");
        return NULL;
    }
    long f_size = ftell(f_test_plan);
    zlog_debug(loggers->common, "Test plan file size is : %li", f_size);    
    rewind(f_test_plan);  
    zlog_info(loggers->common, "Allocating memory for test plan file content.");
    char * file_data = (char *)malloc(sizeof(char) * f_size);
    if(file_data == NULL){        
        zlog_error(loggers->common, "Failed to allocate memory for file.");
        return NULL;
    }  
    
    if(fread(file_data, 1, f_size, f_test_plan) != f_size){        
        zlog_error(loggers->common, "Failed to read configuration file data.");
        free(file_data);
        return NULL;
    }    
    zlog_info(loggers->common, "Closing test plan file");
    if(fclose(f_test_plan) == EOF){
        zlog_error(loggers->common, "Failed to close test plan file");
    }
    return file_data;
}

runner_data * generate_runner_data(char * test_plan_name){
    zlog_info(loggers->common, "Starting reading data from test plan file");
    char * test_plan_file_data = read_test_plan(test_plan_name);
    if(test_plan_file_data == NULL){
        return NULL;
    }
    test_plan t_plan;
    zlog_info(loggers->common, "Starting creating test plan structure from JSON text file.");
    int result = create_test_plan_from_json(test_plan_file_data, &t_plan);
    free(test_plan_file_data);
    if(result){
        return NULL;
    }
    runner_data * r_data = (runner_data*)malloc(sizeof(runner_data));
    if(r_data == NULL){
        zlog_error(loggers->common, "Failed to allocat memory for runner_data structure");
        return NULL;
    }
    r_data->steps_count = t_plan.steps_count;
    if(t_plan.steps_count < 2){
        zlog_error(loggers->common, "Minimal number of steps should be not less then 2. The actual size is: %i", t_plan.steps_count);
        return NULL;
        //TODO: add stop step or run until completion - one iteration
    }
    r_data->start_delay = t_plan.start_delay;
    r_data->steps = (step_data*)malloc(sizeof(step_data) * t_plan.steps_count);
    if(r_data->steps == NULL){
        zlog_error(loggers->common, "Failed to allocate memory for runner_data.steps");
        free_runner_data(r_data);
        return NULL;
    }

    int actions_count = t_plan.actions_count;
    if(actions_count == 0){
        zlog_error(loggers->common, "Test plan actions count is 0.");
        free_runner_data(r_data);
        return NULL;
    }
    action_data * actions = (action_data*)malloc(sizeof(action_data) * actions_count);
    if(actions == NULL){
        zlog_error(loggers->common, "Failed to allocate memory for action_data array");
        free_runner_data(r_data);
        return NULL;
    }
    for(int i = 0; i < actions_count; i++){
        if(get_function_references(&(actions[i]), t_plan.actions[i].action_file_name)){
            free_runner_data(r_data);
            return NULL;
        }
    }

    for(int i = 0; i < t_plan.steps_count; i++){
        r_data->steps[i].next_step_time_interval = (t_plan.steps)[i].run_duration;
        r_data->steps[i].slope_delay = t_plan.steps[i].slope_duration;
        r_data->steps[i].threads_count = t_plan.steps[i].threads_count;
        zlog_debug(loggers->common, "Step № %i: Next step time interval: %li, Slope delay: %li, Threads count: %li.",
                i,
                r_data->steps[i].next_step_time_interval,
                r_data->steps[i].slope_delay,
                r_data->steps[i].threads_count );
        r_data->steps[i].threads_array = (thread_data*)malloc(sizeof(thread_data) * (t_plan.steps)[i].threads_count);
        if(r_data->steps[i].threads_array == NULL){
            zlog_error(loggers->common, "Failed to allocate memory for threads_array");
            free_runner_data(r_data);
            return NULL;
        }
        for(int n = 0; n < r_data->steps[i].threads_count; n++){
            for(int m = 0; m < actions_count; m++){
                thread_data * td = &(r_data->steps[i].threads_array[n]);
                action * act = &(t_plan.actions[m]);
                td->pacing = act->pacing;
                td->run_action_function = define_pacing_function(act->pacing_type);
                td->action = &(actions[m]);
                td->stop_flag = 0;
                td->thread = NULL;
            }
        }
    }    
    return r_data;
}

void free_runner_data(runner_data * run_data){
    zlog_info(loggers->common, "Freing runner data.");
    if(run_data == NULL){
        return;
    }
    if(run_data->steps == NULL){
        return;
    }
    for(long i = 0; i < run_data->steps_count; i++){
        step_data s_data = run_data->steps[i];
        thread_data * threads_array = s_data.threads_array;
        if(threads_array == NULL){
            continue;
        }
        for(long n = 0; n < s_data.threads_count; n++){
            action_data * action = s_data.threads_array[n].action;
            if(action == NULL){
                continue;
            }
            zlog_info(loggers->common, "Freing  scenario library.");
            FreeLibrary(action->library_handler);
            action->action = NULL;
            action->init = NULL;
            action->end = NULL;
        }
        zlog_info(loggers->common, "Freing threads_array memory.");
        free(threads_array);
    }
    zlog_info(loggers->common, "Freing run_data->steps memory.");
    free(run_data->steps);
}

static int create_test_plan_from_json(char * json_string, test_plan * plan){
    void * state;
    test_plan t_plan = *plan;
    int objects_count;
    zlog_info(loggers->common, "Calling UJDecode function");
    UJObject obj =  UJDecode(json_string, strlen(json_string), NULL, &state);
    if(obj == NULL){
        zlog_error(loggers->common, "Failed to parse test plan file.\n%s", UJGetError(state));
        return 1;
    }
    const wchar_t * tree_keys[] = { L"test_name", L"start_delay", L"actions", L"steps"};
    UJObject o_test_name, o_start_delay, o_actions, o_steps;
    objects_count = 4;
    zlog_info(loggers->common, "Calling UJObjectUnpack for parsing main JSON object");
    if(UJObjectUnpack(obj, objects_count, "SNAA", tree_keys, &o_test_name, &o_start_delay, &o_actions, &o_steps) != objects_count){
        zlog_error(loggers->common, "Failed to unpack JSON object");
        UJFree(state);
        return 1;
    }
    zlog_info(loggers->common, "Calling UJReadString for test name");
    t_plan.name = UJReadString(o_test_name, NULL);

    zlog_info(loggers->common, "Calling UJNumericInt for start delay");    
    t_plan.start_delay = UJNumericInt(o_start_delay);

    zlog_info(loggers->common, "Calling UJBeginArray for actions arra");        
    void * iterator = UJBeginArray(o_actions);
    if(iterator == NULL){
        zlog_error(loggers->common, "Failed to create JSON array iterator.");
        UJFree(state);
        return 1;
    }
    UJObject o_action, o_act_file_name, o_pacing, o_percentage;
    const wchar_t * action_keys[] = { L"name", L"pacing", L"percentage" };
    objects_count = 3;
    int i = 1;   
    t_plan.actions = NULL; 
    zlog_info(loggers->common, "Starting cycle of getting actions data to structure");
    while(UJIterArray(&iterator, o_action)){
        t_plan.actions_count = i;
        zlog_info(loggers->common, "Allocating memory for actions array for every one new element.");
        t_plan.actions = (action*)realloc(t_plan.actions, sizeof(action) * i);
        if(t_plan.actions == NULL){    
            zlog_error(loggers->common, "Actions realloc failed");
            UJFree(state);
            return 1;
        }  
        zlog_info(loggers->common, "Calling UJObjectUnpack for action object element.");
        if(UJObjectUnpack(o_action, objects_count, "SNN", action_keys, o_act_file_name, o_pacing, o_percentage) != objects_count){    
            zlog_error(loggers->common, "Failed to unpack JSON action object");
            return 1;            
        }
        zlog_info(loggers->common, "Calling UJReadString for action file name.");
        t_plan.actions[i].action_file_name = UJReadString(o_act_file_name, NULL);
        if(t_plan.actions[i].action_file_name == NULL){            
            zlog_error(loggers->common, "Failed to get action file name");
            UJFree(state);
            return 1;
        }
        zlog_info(loggers->common, "Calling UJNumericInt for pacint.");
        t_plan.actions[i].pacing = UJNumericInt(o_pacing);
        zlog_info(loggers->common, "Calling UJNumericInt for percentage");
        t_plan.actions[i].percentage = UJNumericInt(o_percentage);     
        i++;
    }

    zlog_info(loggers->common, "Actions successfully parsed. Starting UJBeginArray for steps array.");
    iterator = UJBeginArray(o_steps);
    if(iterator == NULL){        
        zlog_error(loggers->common, "Failed to get steps array iterator");
        UJFree(state);
        return 1;
    }

    UJObject o_step, o_step_type, o_step_run_duration, o_step_threads_count, o_step_slope_duration;
    objects_count = 4;
    i = 0;
    const wchar_t * step_keys[] = { L"name", L"action_type", L"run_duration", L"threads_count", L"slope_duration"};
    zlog_info(loggers->common, "Starting cycle of getting steps data to structure");
    while(UJIterArray(iterator, o_step)){
        t_plan.steps_count = i;
        zlog_info(loggers->common, "Allocating memory for steps array for every one new element.");
        t_plan.steps = (test_step*)realloc(t_plan.steps, sizeof(test_step) * i);
        if(t_plan.steps == NULL){            
            zlog_error(loggers->common, "Steps realloc failed");
            UJFree(state);
            return 1;
        }
        zlog_info(loggers->common, "Calling UJObjectUnpack for step object element.");
        if(UJObjectUnpack(o_steps, objects_count, "SNNN", step_keys, o_step_type, o_step_run_duration, o_step_threads_count, o_step_slope_duration) != objects_count){            zlog_error(loggers->common, "Failed to unpack JSON step object");
            UJFree(state);
            return 1;
        }
        size_t str_size = 0;
        zlog_info(loggers->common, "Calling UJReadString for step type name.");
        const wchar_t * act_type = UJReadString(o_step_type, &str_size);
        t_plan.steps[i].step_type = (wchar_t *)malloc(sizeof(wchar_t) * (str_size + 1));
        if(t_plan.steps[i].step_type == NULL){            
            zlog_error(loggers->common, "Step_type malloc failed");
            return 1;
        }
        wcscpy(t_plan.steps[i].step_type, act_type);        
        zlog_info(loggers->common, "Calling UJNumericInt for step duration");
        t_plan.steps[i].run_duration = UJNumericInt(o_step_run_duration);

        zlog_info(loggers->common, "Calling UJNumericInt for step threads count");
        t_plan.steps[i].threads_count = UJNumericInt(o_step_threads_count);
        
        zlog_info(loggers->common, "Calling UJNumericInt for step slope duration");
        t_plan.steps[i].slope_duration = UJNumericInt(o_step_slope_duration);
        i++;
    }
    zlog_info(loggers->common, "Calling UJFree.");
    UJFree(state);
    return 0;
}

static int get_function_references(action_data * p_action, const wchar_t * file_name){
    int fname_len = wcslen(file_name);
    char * f_name = (char*)malloc(sizeof(char) * (fname_len + 1));
    for(int i = 0; i < fname_len; i++){
        f_name[i] = wctob(file_name[i]);
    }
    f_name[fname_len] = '\0';

    p_action->library_handler = LoadLibrary(f_name);
    if(p_action->library_handler == NULL){        
        zlog_error(loggers->common, "Failed to load file %s. [%s]", f_name, GetLastError());
        return 1;
    }
    p_action->action = (void*)GetProcAddress(p_action->library_handler, "action");
    if(p_action->action == NULL){        
        zlog_error(loggers->common, "Failde to get \"action\" method from file: %s", f_name);
        FreeLibrary(p_action->library_handler);
        return 1;
    }
    p_action->init = (void*)GetProcAddress(p_action->library_handler, "init");
    p_action->end = (void*)GetProcAddress(p_action->library_handler, "end");
}

static LPTHREAD_START_ROUTINE define_pacing_function(const wchar_t * pacing_type){
    if(wcscmp(pacing_type, L"no")){
        return no_pacing;
    }
    if(wcscmp(pacing_type, L"fixed")){
        return fixed_pacing;
    }
    if(wcscmp(pacing_type, L"relative")){
        return relative_pacing;
    }
    zlog_error(loggers->common, "Specified pacing type \"%ws\"is not recognized. It should be :\"no\", \"fixed\", \"relative\"", pacing_type);
    return NULL;
}
