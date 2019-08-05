#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#include "test_plan.h"
#include "logger.h"


size_t save_response_data(void*, size_t, size_t, void *);
size_t extract_listing_href(char *, char ***);
size_t read_shop_links_from_file(FILE *, char ***);

void print_help();

typedef struct action_statistics{
    wchar_t * name;
    unsigned int running;
    unsigned int pending_run;
    unsigned int stopped;
    unsigned int pending_stop;
    unsigned int failed;
    unsigned int thr_count;
    unsigned int thr_run;
} action_stats_data;

typedef struct actions_statistics{
    action_stats_data * statistics;
    unsigned int count;
} actions_stats_data;
