typedef void * (*p_pacing_function)(void*);

void *  no_pacing(void *);
void *  fixed_pacing(void *);
void *  relative_pacing(void *);