#include <wchar.h>
#include "common_types.h"

extern unsigned int init_failed;

EXPORT void debug_message(const wchar_t * text, ...);

EXPORT void info_message(const wchar_t * text, ...);

EXPORT void warning_message(const wchar_t * text, ...);

EXPORT void error_message(const wchar_t * text, ...);

EXPORT void fatal_message(const wchar_t * text, ...);