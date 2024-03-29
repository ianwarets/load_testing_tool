#include "test_plan.h"
#include "ujdecode.h"
#include "logger.h"
#include "action_wrappers.h"
#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <string.h>

static int create_test_plan_from_json(char *, test_plan *);
static p_pacing_function define_pacing_function(wchar_t *);
static int save_json_string(char *, UJObject , wchar_t **);
static int get_function_references(action_data *, wchar_t *);
static int generate_actions_data(runner_data * r_data, test_plan t_plan);
static wchar_t * get_action_name(wchar_t * action_file_name);

/// @brief 
/// @param test_plan_name 
/// @return 
static char * read_test_plan(char * test_plan_name){
    FILE * f_test_plan = fopen(test_plan_name, "r");
    if(f_test_plan == NULL){
        error_message("Failed to open test plan file. '%s'", test_plan_name);
        return NULL;
    }
    if(fseek(f_test_plan, 0, SEEK_END)){        
        error_message("fseek failed to move to the end of file.");
        return NULL;
    }
    long f_size = ftell(f_test_plan);
#ifdef DEBUG
    debug_message("Test plan file size is : %li", f_size);    
#endif
    rewind(f_test_plan);  
    char * file_data = (char *)malloc(sizeof(char) * (f_size + 1));
    if(file_data == NULL){        
        error_message("Failed to allocate memory for file.");
        return NULL;
    }  
    size_t fread_result = fread(file_data, 1, f_size, f_test_plan);
    if(fread_result != f_size && feof(f_test_plan) == 0 && ferror(f_test_plan) > 0){        
        error_message("Failed to read configuration file data.");
        free(file_data);
        return NULL;
    }    
    file_data[fread_result] = '\0';
    if(fclose(f_test_plan) == EOF){
        error_message("Failed to close test plan file.");
    }
    return file_data;
}

/// @brief 
/// @param test_plan_name 
/// @return 
runner_data * generate_runner_data(char * test_plan_name){
    char * test_plan_file_data = read_test_plan(test_plan_name);
    if(test_plan_file_data == NULL){
        return NULL;
    }
    test_plan t_plan = {};
    int result = create_test_plan_from_json(test_plan_file_data, &t_plan);
    free(test_plan_file_data);
    if(result){
        return NULL;
    }
    runner_data * r_data = (runner_data*)malloc(sizeof(runner_data));

    if(r_data == NULL){
        error_message("Failed to allocate memory for runner_data structure.");
        return NULL;
    }

    r_data->actions = NULL;
    r_data->actions_count = 0;
    r_data->total_threads_count = 0;
    r_data->start_delay = 0;
    r_data->steps = NULL;
    r_data->steps_count = 0;    

    r_data->steps_count = t_plan.steps_count;
    if(t_plan.steps_count < 2){
        error_message("Minimal number of steps should be not less then 2. The actual size is: %i.", t_plan.steps_count);
        return NULL;
        //TODO: add stop step or run until completion - one iteration
    }
    r_data->start_delay = t_plan.start_delay;
    r_data->steps = (step_data*)malloc(sizeof(step_data) * t_plan.steps_count);
    if(r_data->steps == NULL){
        error_message("Failed to allocate memory for runner_data.steps.");
        free_runner_data(r_data);
        return NULL;
    }    
    
    for(int i = 0; i < t_plan.steps_count; i++){
        r_data->steps[i].duration = t_plan.steps[i].run_duration;
        r_data->steps[i].slope_delay = t_plan.steps[i].slope_duration;
        r_data->steps[i].threads_count = t_plan.steps[i].threads_count;
        // In case step type is equal to "start" then step will start threads, else in will be considered as stop threads step
        r_data->steps[i].to_start = wcscmp(t_plan.steps[i].step_type, L"start") == 0 ? true : false;
        if(r_data->steps[i].to_start){
            r_data->total_threads_count += r_data->steps[i].threads_count;
        }
        r_data->steps[i].r_data = r_data;
        r_data->steps[i].step_index = i;
#ifdef DEBUG
        debug_message("Step #%i: %s, Next step time interval: %li, Slope delay: %li, Threads count: %li.",
                i,
                r_data->steps[i].to_start ? "Start" : "Stop",
                r_data->steps[i].duration,
                r_data->steps[i].slope_delay,
                r_data->steps[i].threads_count );
#endif
    }   

    if(generate_actions_data(r_data, t_plan) != 0){
        free_runner_data(r_data);
        return NULL;
    }
       
    return r_data;
}

/// @brief 
/// @param run_data 
void free_runner_data(runner_data * run_data){
    if(run_data == NULL){
        return;
    }    
    if(run_data->actions == NULL){
        return;
    }
    for(long i = 0; i < run_data->actions_count; i++){
        action_data action = run_data->actions[i];
        if(action.action_lib_handler != NULL){
            dlclose(action.action_lib_handler);
        }
        action.action = NULL;
        action.init = NULL;
        action.end = NULL;

        thread_data * threads_array = action.threads;
        if(threads_array == NULL){
            continue;
        }
        free(threads_array);
    }
    free(run_data->steps);
}

/// @brief 
/// @param json_string 
/// @param t_plan 
/// @return 
static int create_test_plan_from_json(char * json_string, test_plan * t_plan){
    void * state;
    int objects_count;
    UJObject obj =  UJDecode(json_string, strlen(json_string), NULL, &state);
    if(obj == NULL){
        error_message("Failed to parse test plan file.\n\t%s.", UJGetError(state));
        return 1;
    }
    const wchar_t * tree_keys[] = { L"test_name", L"start_delay", L"actions", L"steps"};
    UJObject    o_test_name = NULL,
                o_start_delay = NULL,
                o_actions = NULL,
                o_steps = NULL;

    objects_count = 4;
    if(UJObjectUnpack(obj, objects_count, "SNAA", tree_keys, &o_test_name, &o_start_delay, &o_actions, &o_steps) != objects_count){
        error_message("Failed to unpack JSON object.");
        UJFree(state);
        return 1;
    }

    if(save_json_string("test name", o_test_name, &(*t_plan).name) > 0){
        UJFree(state);
        return 1;
    }
   
    (*t_plan).start_delay = UJNumericInt(o_start_delay);
       
    void * iterator = UJBeginArray(o_actions);
    if(iterator == NULL){
        error_message("Failed to create JSON array iterator.");
        UJFree(state);
        return 1;
    }
    UJObject    o_action = NULL, 
                o_act_file_name = NULL,
                o_pacing = NULL,
                o_pacing_type = NULL,
                o_percentage = NULL;

    const wchar_t * action_keys[] = { L"name", L"pacing", L"pacing_type", L"percentage" };
    objects_count = 4;
    int i = 0;   
    (*t_plan).actions = NULL; 
    while(UJIterArray(&iterator, &o_action)){
        (*t_plan).actions_count = i + 1;
        (*t_plan).actions = (action*)realloc((*t_plan).actions, sizeof(action) * (*t_plan).actions_count);
        if((*t_plan).actions == NULL){    
            error_message("Actions realloc failed.");
            UJFree(state);
            return 1;
        }  
        if(UJObjectUnpack(o_action, objects_count, "SNSN", action_keys, &o_act_file_name, &o_pacing, &o_pacing_type, &o_percentage) != objects_count){    
            error_message("Failed to unpack JSON action object.");
            return 1;            
        }

        if(save_json_string("action file name", o_act_file_name, &(*t_plan).actions[i].action_file_name) > 0){
            UJFree(state);
            return 1;
        }

        (*t_plan).actions[i].pacing = UJNumericInt(o_pacing);        

        if(save_json_string("pacing type", o_pacing_type, &(*t_plan).actions[i].pacing_type) > 0){
            UJFree(state);
            return 1;
        }

        (*t_plan).actions[i].percentage = UJNumericInt(o_percentage);     
        i++;
    }

    iterator = UJBeginArray(o_steps);
    if(iterator == NULL){        
        error_message("Failed to get steps array iterator.");
        UJFree(state);
        return 1;
    }

    UJObject    o_step = NULL,
                o_step_type = NULL,
                o_step_run_duration = NULL,
                o_step_threads_count = NULL,
                o_step_slope_duration = NULL;

    objects_count = 4;
    i = 0;
    const wchar_t * step_keys[] = {L"action_type", L"run_duration", L"threads_count", L"slope_duration"};
    (*t_plan).steps = NULL;
    while(UJIterArray(&iterator, &o_step)){
        (*t_plan).steps_count = i + 1;
        (*t_plan).steps = (test_step*)realloc((*t_plan).steps, sizeof(test_step) * (*t_plan).steps_count);
        if((*t_plan).steps == NULL){            
            error_message("Steps realloc failed");
            UJFree(state);
            return 1;
        }
        if(UJObjectUnpack(o_step, objects_count, "SNNN", step_keys, &o_step_type, &o_step_run_duration, &o_step_threads_count, &o_step_slope_duration) != objects_count){            
            error_message("Failed to unpack JSON step object");
            UJFree(state);
            return 1;
        }
        if(save_json_string("step type", o_step_type, &(*t_plan).steps[i].step_type) > 0){
            UJFree(state);
            return 1;
        }
     
        (*t_plan).steps[i].run_duration = UJNumericInt(o_step_run_duration);

        (*t_plan).steps[i].threads_count = UJNumericInt(o_step_threads_count);
        
        (*t_plan).steps[i].slope_duration = UJNumericInt(o_step_slope_duration);
        i++;
    }
    UJFree(state);
    return 0;
}

static int get_function_references(action_data * p_action, wchar_t * file_name){
    int fname_len = wcslen(file_name);
    debug_message("File to open length '%i' is wchar_t:'%ls'", fname_len, file_name);
    char * f_name = (char*)malloc(sizeof(char) * (fname_len + 1));
    wcstombs(f_name, file_name, fname_len);   
    f_name[fname_len] = '\0';
    debug_message("File to open is char:\"%s\"", f_name); 
    char * err_msg_buf;
    p_action->action_lib_handler = dlopen(f_name, RTLD_LAZY);     
    if(p_action->action_lib_handler == NULL){        
        err_msg_buf = dlerror();    
        error_message("Failed to load file %s. [%s].", f_name, err_msg_buf);
        free(f_name);
        return 1;
    }
    p_action->action = (void*)dlsym(p_action->action_lib_handler, "action");
    err_msg_buf = dlerror();
    if(p_action->action == NULL && err_msg_buf != NULL){        
        error_message("Failed to get \"action\" method from file: %s. %s", f_name, err_msg_buf);
        dlclose(p_action->action_lib_handler);
        free(f_name);
        return 1;
    }
    p_action->init = (void*)dlsym(p_action->action_lib_handler, "init");
    err_msg_buf = dlerror();
    if(p_action->init == NULL && err_msg_buf != NULL){        
        error_message("Failed to get \"init\" method from file: %s. %s", f_name, err_msg_buf);
        dlclose(p_action->action_lib_handler);
        free(f_name);
        return 1;
    }
    p_action->end = (void*)dlsym(p_action->action_lib_handler, "end");
    err_msg_buf = dlerror();
    if(p_action->end == NULL && err_msg_buf != NULL){        
        error_message("Failed to get \"end\" method from file: %s. %s", f_name, err_msg_buf);
        dlclose(p_action->action_lib_handler);
        free(f_name);
        return 1;
    }
    free(f_name);
    return 0;
}

static p_pacing_function define_pacing_function(wchar_t * pacing_type){
    if(wcscmp(pacing_type, L"no") == 0){
        return no_pacing;
    }
    if(wcscmp(pacing_type, L"fixed") == 0){
        return fixed_pacing;
    }
    if(wcscmp(pacing_type, L"relative") == 0){
        return relative_pacing;
    }
    error_message("Specified pacing type \"%ws\"is not recognized. It should be :\"no\", \"fixed\", \"relative\".", pacing_type);
    return NULL;
}

static int save_json_string(char * variable_name, UJObject ujobject, wchar_t ** output_string){
    size_t size = 0;
    const wchar_t * step_type = UJReadString(ujobject, &size);
    if(size == 0){
        error_message("Failed to read string for %s.", variable_name);
        return 1;
    }
    *output_string = (wchar_t*)malloc(sizeof(wchar_t) * (size + 1));
    if(*output_string == NULL){
        error_message("Failed to allocate memory for %s.", variable_name);
        return 1;
    }
    wcscpy(*output_string, step_type);
    return 0;
}

static int generate_actions_data(runner_data * r_data, test_plan t_plan){
    int actions_count = t_plan.actions_count;
    r_data->actions_count = actions_count;
    if(actions_count == 0){
        error_message("Test plan actions count is 0.");        
        return 1;
    }
    action_data * actions = (action_data*)malloc(sizeof(action_data) * actions_count);
    if(actions == NULL){
        error_message("Failed to allocate memory for action_data array.");
        return 1;
    }
    r_data->actions = actions;
    for(int i = 0; i < actions_count; i++){
        actions[i].threads = NULL;
        actions[i].threads_count = 0;
        actions[i].running_threads = 0;
        actions[i].init = NULL;
        actions[i].action = NULL;
        actions[i].end = NULL;
        actions[i].name = NULL;
        actions[i].action_lib_handler = NULL;
        actions[i].runner = NULL;
        actions[i].pacing = 0;
        actions[i].ratio = 0;
        
        if(get_function_references(&actions[i], t_plan.actions[i].action_file_name)){
            return 1;
        }
        actions[i].ratio = t_plan.actions[i].percentage;
        actions[i].runner = define_pacing_function(t_plan.actions[i].pacing_type);
        if(actions[i].runner == NULL){
            error_message("Failed to detect pacing type.");
            return 1;
        }
        actions[i].pacing = t_plan.actions[i].pacing;

        unsigned int th_count = (r_data->total_threads_count * actions[i].ratio) / 100;
        if(th_count < 1){
            th_count = 1;
        }
        actions[i].threads_count = th_count;     
        actions[i].threads = (thread_data *)malloc(sizeof(thread_data) * th_count);
        if(actions[i].threads == NULL){
            error_message("Failed to allocate memory for threads array for action # %i", i);
            return 1;
        }
        for(int t = 0; t < th_count; t++){
            actions[i].threads[t].handle = 0;
            actions[i].threads[t].action = NULL;
            actions[i].threads[t].index = 0;
            actions[i].threads[t].stop_thread = 0;
            
        }
        actions[i].running_threads = 0;  
        actions[i].runner = define_pacing_function(t_plan.actions[i].pacing_type);
        if(get_function_references(&actions[i], t_plan.actions[i].action_file_name) != 0){
            return 1;
        }
        actions[i].name = get_action_name(t_plan.actions[i].action_file_name);
    }
    return 0;
}

static wchar_t * get_action_name(wchar_t * action_file_name){
    wchar_t * result = wcsrchr(action_file_name, L'/');
    if(result == NULL){
        return action_file_name;
    }
    return ++result;
}