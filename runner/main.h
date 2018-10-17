#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "test_plan.h"
#include "logger.h"

size_t save_response_data(void*, size_t, size_t, void *);
size_t extract_listing_href(char *, char ***);
size_t read_shop_links_from_file(FILE *, char ***);

void print_help();