typedef struct {
    char * action_file_name;
    long pacing;
    long percentage;
} action;

typedef struct {
    char * action_type;
    long run_duration;
    long thread_count;
    long slope_duration;
} test_step;

typedef struct {
    char * name;
    test_step * steps;
    action * actions;
} tesp_plan;