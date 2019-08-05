#include "logger.h"
#include <stdlib.h>

static zlog_categories * loggers = NULL;

// Выполнить однократнуню инициализацию.
int logger_init(){	
	if(loggers != NULL){
		return 0;
	}
	loggers = (typeof(loggers))malloc(sizeof(typeof(*loggers)));
	zlog_category_t * zlog_scenario;
	zlog_category_t * zlog_statistics;
	zlog_category_t * zlog_common;
	zlog_category_t * zlog_transactions;

	// Set the ZLOG_CONF_PATH environment variable to address the configuration file
	char * zlog_conf_path = getenv("ZLOG_CONF_PATH");
	fprintf(stdout, "Log file configuration file path: %s\n", zlog_conf_path);
	int rc = zlog_init(zlog_conf_path);
	free(zlog_conf_path);
	if(rc){
		fprintf(stderr, "Zlog init failed\nSee log file at: %s\n", getenv("ZLOG_PROFILE_ERROR"));
		return 1;
	}
	zlog_scenario = zlog_get_category("scenario");
	zlog_statistics = zlog_get_category("statistics");
	zlog_common = zlog_get_category("common");
	zlog_transactions = zlog_get_category("transactions");

	if(!zlog_scenario || !zlog_statistics || !zlog_common || !zlog_transactions){
		if(!zlog_statistics){
			fprintf(stderr, "Get \"statistics\" zlog category failed.\n");
		}
		if(!zlog_scenario){
			fprintf(stderr, "Get \"programm_log\" zlog category failed.\n");
		}
		if(!zlog_common){
			fprintf(stderr, "Get \"common\" zlog category failed.\n");
		}
		if(!zlog_transactions){
			fprintf(stderr, "Get \"transactions\" zlog category failed.\n");
		}
		zlog_fini();
		return 1;
	}

	loggers->common = zlog_common;
	loggers->scenario = zlog_scenario;
	loggers->statistics = zlog_statistics;
	loggers->transactions = zlog_transactions;
#ifdef DEBUG
	zlog_debug(zlog_common, "Logger created successfully.\n");
#endif
	return 0;
}

zlog_categories * logger_get_loggers(){
	if(loggers == NULL){
		fprintf(stderr, "Loggers are not initialized.\n");		
	}
	return loggers;
}

void logger_close(){
	free(loggers);
	zlog_fini();
}
