#include <wchar.h>

extern unsigned int init_failed;

void debug_message(const wchar_t * text, ...);

void info_message(const wchar_t * text, ...);

void warning_message(const wchar_t * text, ...);

void error_message(const wchar_t * text, ...);

void fatal_message(const wchar_t * text, ...);