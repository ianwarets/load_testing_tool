#include <zlog.h>

#pragma once
typedef struct {
    zlog_category_t * scenario;
    zlog_category_t * statistics;
    zlog_category_t * transactions;
    zlog_category_t * common; 
} zlog_categories;

/**
 * Выполняет инициализацию библиотеки журналирования.
 * Вызывать в основном потоке один раз.
 * Не многопоточная функция
 */
int logger_init();

/**
 * Получение структуры с журналировщиками.
 * @return структура с жарналировщиками для всего приложения.
 */
zlog_categories * logger_get_loggers();

/**
 * Завершает работу библиотеки журналирования.
 * Вызывать в основном потоке.
 */
void logger_close();