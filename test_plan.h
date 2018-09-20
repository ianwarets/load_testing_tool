#include "test_controller.h"

typedef struct {
    wchar_t * action_file_name;
    long pacing;
    long percentage;
} action;

typedef struct {
    wchar_t * action_type;
    long run_duration;
    long threads_count;
    long slope_duration;
} test_step;

typedef struct {
    wchar_t * name;
    test_step * steps;
    int steps_count;
    action * actions;
    int actions_count;
} test_plan;


void free_runner_data(runner_data *, void(*)());
runner_data * generate_runner_data(char *);
char * read_test_plan(char *);