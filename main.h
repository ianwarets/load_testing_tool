#include <zlog.h>

typedef struct {
    zlog_category_t * programm_category;
    zlog_category_t * statistics;
    zlog_category_t * common; 
} zlog_categories;

typedef struct {
    void (*init)();
    void (*action)();
    void (*end)();
    void * (*shared_data_init)();
} action_functions;