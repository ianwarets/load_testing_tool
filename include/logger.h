#include <wchar.h>

extern unsigned int init_failed;

void debug_message(const char * text, ...);

void info_message(const char * text, ...);

void warning_message(const char * text, ...);

void error_message(const char * text, ...);

void fatal_message(const char * text, ...);