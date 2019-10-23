#include "logger.h"
#include <stdlib.h>
#include <stdarg.h>
#include <GTypes.h>
#include <P7_Cproxy.h>


unsigned int init_failed = 0;
static hP7_Client g_hClient = NULL;
static hP7_Trace g_hTrace = NULL;

// Выполнить однократнуню инициализацию.
static __attribute__((constructor)) void logger_init(){	
	g_hClient = P7_Client_Create(TM("/P7.Sink=Auto /P7.Addr=127.0.0.1 /P7.Pool=16000"));
	g_hTrace = P7_Trace_Create(g_hClient, TM("RevoloaderTraceChannel"), NULL);
}

static __attribute__((destructor)) void logger_close(){

}

void debug_message(const char * text, ...){
	va_list args;
	va_start(args, text);
	P7_Trace_Embedded(g_hTrace, 0, P7_TRACE_LEVEL_DEBUG, NULL, (tUINT16)__LINE__, __FILE__, __FUNCTION__, &text, &args);
	va_end(args);
}

void info_message(const char * text, ...){
	va_list args;
	va_start(args, text);
	P7_Trace_Embedded(g_hTrace, 0, P7_TRACE_LEVEL_INFO, NULL, (tUINT16)__LINE__, __FILE__, __FUNCTION__, &text, &args);
	va_end(args);
}

void warning_message(const char * text, ...){
	va_list args;
	va_start(args, text);
	P7_Trace_Embedded(g_hTrace, 0, P7_TRACE_LEVEL_WARNING, NULL, (tUINT16)__LINE__, __FILE__, __FUNCTION__, &text, &args);
	va_end(args);
}

void error_message(const char * text, ...){
	va_list args;
	va_start(args, text);
	P7_Trace_Embedded(g_hTrace, 0, P7_TRACE_LEVEL_ERROR, NULL, (tUINT16)__LINE__, __FILE__, __FUNCTION__, &text, &args);
	va_end(args);
}

void fatal_message(const char * text, ...){
	va_list args;
	va_start(args, text);
	P7_Trace_Embedded(g_hTrace, 0, P7_TRACE_LEVEL_CRITICAL, NULL, (tUINT16)__LINE__, __FILE__, __FUNCTION__, &text, &args);
	va_end(args);
}
