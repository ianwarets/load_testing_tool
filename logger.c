#include "logger.h"
#include <stdlib.h>
#include <stdarg.h>
#include "GTypes.h"
#include "P7_Cproxy.h"
#include <stdio.h>

static hP7_Client g_hClient = NULL;
static hP7_Trace g_hTrace = NULL;

// Выполнить однократнуню инициализацию.
static __attribute__((constructor)) void logger_init(){	
	P7_Set_Crash_Handler();
	g_hClient = P7_Client_Create(TM("/P7.Sink=FileTxt"));
	if(g_hClient == NULL){
		printf("Failed to create P7 Client");
		return;
	}
	g_hTrace = P7_Trace_Create(g_hClient, TM("RevoloaderTraceChannel"), NULL);
	if(g_hTrace == NULL){
		printf("Failed to create Trace ");
		return;
	}
}

static __attribute__((destructor)) void logger_close(){
	P7_Client_Flush(g_hClient);

	if (g_hTrace)
    {
        P7_Trace_Release(g_hTrace);
        g_hTrace = NULL;
    }

	if (g_hClient)
    {
        P7_Client_Release(g_hClient);
        g_hClient = NULL;
    }
	P7_Clr_Crash_Handler();
	printf("Destructor called successfully.\n");
}

/// @brief 
/// @param text 
/// @param  
void debug_message(const tXCHAR * text, ...){
	va_list args;
	va_start(args, text);
	P7_Trace_Embedded(g_hTrace, 0, P7_TRACE_LEVEL_DEBUG, NULL, (tUINT16)__LINE__, __FILE__, __FUNCTION__, &text, &args);
	va_end(args);
}

/// @brief 
/// @param text 
/// @param  
void info_message(const tXCHAR * text, ...){
	va_list args;
	va_start(args, text);
	P7_Trace_Embedded(g_hTrace, 0, P7_TRACE_LEVEL_INFO, NULL, (tUINT16)__LINE__, __FILE__, __FUNCTION__, &text, &args);
	va_end(args);
}

/// @brief 
/// @param text 
/// @param  
void warning_message(const tXCHAR * text, ...){
	va_list args;
	va_start(args, text);
	P7_Trace_Embedded(g_hTrace, 0, P7_TRACE_LEVEL_WARNING, NULL, (tUINT16)__LINE__, __FILE__, __FUNCTION__, &text, &args);
	va_end(args);
}

/// @brief 
/// @param text 
/// @param  
void error_message(const tXCHAR * text, ...){
	va_list args;
	va_start(args, text);
	P7_Trace_Embedded(g_hTrace, 0, P7_TRACE_LEVEL_ERROR, NULL, (tUINT16)__LINE__, __FILE__, __FUNCTION__, &text, &args);
	va_end(args);
}

/// @brief 
/// @param text 
/// @param  
void fatal_message(const tXCHAR * text, ...){
	va_list args;
	va_start(args, text);
	P7_Trace_Embedded(g_hTrace, 0, P7_TRACE_LEVEL_CRITICAL, NULL, (tUINT16)__LINE__, __FILE__, __FUNCTION__, &text, &args);
	va_end(args);
}
