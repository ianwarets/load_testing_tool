#include "logger.h"
#include <zlog.h>

zlog_categories * loggers = NULL;

// Выполнить однократнуню инициализацию.
int logger_init(){
	if(loggers != NULL){
		return 0;
	}
	loggers = (typeof(loggers))malloc(sizeof(typeof(*loggers)));
	zlog_category_t * zlog_scenario;
	zlog_category_t * zlog_statistics;
	zlog_category_t * zlog_common;

	int rc = zlog_init("zlog.conf");
	if(rc){
		fprintf(stderr, "zlog init failde\n");
		return 1;
	}

	zlog_scenario = zlog_get_category("scenario");
	zlog_statistics = zlog_get_category("statistics");
	zlog_common = zlog_get_category("common");
	if(!zlog_scenario || !zlog_statistics || !zlog_common){
		if(!zlog_statistics){
			fprintf(stderr, "Get \"statistics\" zlog category failed.\n");
		}
		if(!zlog_scenario){
			fprintf(stderr, "Get \"programm_log\" zlog category failed.\n");
		}
		if(!zlog_common){
			fprintf(stderr, "Get \"common\" zlog category failed.\n");
		}
		zlog_fini();
		return 1;
	}

	loggers->common = zlog_common;
	loggers->scenario = zlog_scenario;
	loggers->statistics = zlog_statistics;
	return 0;
}

int logger_close(){
	free(loggers);
	zlog_fini();
	return 0;
}