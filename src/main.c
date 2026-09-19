#include "calc.h"

#include <stdio.h>

enum {
    CALC_EXIT_SUCCESS = 0,
    CALC_EXIT_USAGE = 2,
    CALC_EXIT_SYNTAX = 3,
    CALC_EXIT_DIVISION_BY_ZERO = 4,
    CALC_EXIT_RANGE = 5
};

static int exit_code_for(calc_status status) {
    switch (status) {
    case CALC_OK:
        return CALC_EXIT_SUCCESS;
    case CALC_ERROR_DIVISION_BY_ZERO:
        return CALC_EXIT_DIVISION_BY_ZERO;
    case CALC_ERROR_RANGE:
        return CALC_EXIT_RANGE;
    case CALC_ERROR_INVALID_ARGUMENT:
    case CALC_ERROR_SYNTAX:
    default:
        return CALC_EXIT_SYNTAX;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: calc \"EXPRESSION\"\n");
        return CALC_EXIT_USAGE;
    }

    calc_result result = calc_evaluate(argv[1]);
    if (result.status != CALC_OK) {
        fprintf(stderr, "calc: %s at byte %zu\n", calc_status_message(result.status),
                result.error_offset);
        return exit_code_for(result.status);
    }

    printf("%.17g\n", result.value);
    return CALC_EXIT_SUCCESS;
}
