#include "ltt_common.h"

unsigned long long get_time_difference(struct timespec start, struct timespec stop){
    unsigned long long diff = stop.tv_sec = start.tv_sec;
    diff *= 1000000000;
    diff = diff + stop.tv_nsec - start.tv_nsec;
    return diff;
}