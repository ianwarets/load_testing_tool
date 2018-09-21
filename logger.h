
typedef struct {
    zlog_category_t * scenario;
    zlog_category_t * statistics;
    zlog_category_t * common; 
} zlog_categories;

int logger_init(zlog_categories *);