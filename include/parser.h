#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

int evaluate_expression(const char *expr, int *result, char *error_msg, size_t error_msg_sz);

#endif
