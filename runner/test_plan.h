#include "test_controller.h"
#include <wchar.h>

typedef struct {
    wchar_t * action_file_name;    
    long pacing;
    wchar_t * pacing_type;
    long percentage;
} action;

typedef struct {
    wchar_t * step_type;
    long run_duration;
    long threads_count;
    long slope_duration;
} test_step;

typedef struct {
    wchar_t * name;
    int start_delay;
    test_step * steps;
    int steps_count;
    action * actions;
    int actions_count;
} test_plan;

runner_data * generate_runner_data(char *);
void free_runner_data(runner_data *);
