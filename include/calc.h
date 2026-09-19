#ifndef CALC_H
#define CALC_H

#include <stddef.h>

typedef enum {
    CALC_OK = 0,
    CALC_ERROR_INVALID_ARGUMENT,
    CALC_ERROR_SYNTAX,
    CALC_ERROR_DIVISION_BY_ZERO,
    CALC_ERROR_RANGE
} calc_status;

typedef struct {
    calc_status status;
    double value;
    size_t error_offset;
} calc_result;

/** Evaluate one NUL-terminated arithmetic expression. */
calc_result calc_evaluate(const char *expression);

/** Return a stable, human-readable description of a status code. */
const char *calc_status_message(calc_status status);

#endif
