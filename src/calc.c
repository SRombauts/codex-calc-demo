#include "calc.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

enum { CALC_MAX_DEPTH = 128 };

typedef struct {
    const char *input;
    const char *cursor;
    calc_status status;
    size_t error_offset;
} parser;

static double parse_expression(parser *state, unsigned int depth);

static void skip_whitespace(parser *state) {
    while (isspace((unsigned char)*state->cursor) != 0) {
        ++state->cursor;
    }
}

static void set_error(parser *state, calc_status status, const char *position) {
    if (state->status == CALC_OK) {
        state->status = status;
        state->error_offset = (size_t)(position - state->input);
    }
}

static int enter_nested_expression(parser *state, unsigned int depth) {
    if (depth >= CALC_MAX_DEPTH) {
        set_error(state, CALC_ERROR_SYNTAX, state->cursor);
        return 0;
    }
    return 1;
}

static double checked_result(parser *state, double value, const char *position) {
    if (!isfinite(value)) {
        set_error(state, CALC_ERROR_RANGE, position);
        return 0.0;
    }
    return value;
}

static double parse_number(parser *state) {
    char *end = NULL;
    const char *start = state->cursor;

    if (isdigit((unsigned char)*start) == 0 &&
        !(*start == '.' && isdigit((unsigned char)start[1]) != 0)) {
        set_error(state, CALC_ERROR_SYNTAX, start);
        return 0.0;
    }

    errno = 0;
    double value = strtod(start, &end);
    state->cursor = end;
    if (errno == ERANGE || !isfinite(value)) {
        set_error(state, CALC_ERROR_RANGE, start);
        return 0.0;
    }
    return value;
}

static double parse_primary(parser *state, unsigned int depth) {
    skip_whitespace(state);
    if (*state->cursor != '(') {
        return parse_number(state);
    }

    const char *opening = state->cursor;
    ++state->cursor;
    if (!enter_nested_expression(state, depth)) {
        return 0.0;
    }

    double value = parse_expression(state, depth + 1U);
    if (state->status != CALC_OK) {
        return 0.0;
    }
    skip_whitespace(state);
    if (*state->cursor != ')') {
        set_error(state, CALC_ERROR_SYNTAX, *state->cursor == '\0' ? state->cursor : opening);
        return 0.0;
    }
    ++state->cursor;
    return value;
}

static double parse_unary(parser *state, unsigned int depth) {
    skip_whitespace(state);
    if (*state->cursor != '+' && *state->cursor != '-') {
        return parse_primary(state, depth);
    }

    const char operation = *state->cursor;
    const char *position = state->cursor;
    ++state->cursor;
    if (!enter_nested_expression(state, depth)) {
        return 0.0;
    }

    double value = parse_unary(state, depth + 1U);
    if (state->status != CALC_OK || operation == '+') {
        return value;
    }
    return checked_result(state, -value, position);
}

static double parse_term(parser *state, unsigned int depth) {
    double value = parse_unary(state, depth);
    while (state->status == CALC_OK) {
        skip_whitespace(state);
        if (*state->cursor != '*' && *state->cursor != '/') {
            break;
        }

        const char operation = *state->cursor;
        const char *position = state->cursor;
        ++state->cursor;
        double right = parse_unary(state, depth);
        if (state->status != CALC_OK) {
            return 0.0;
        }
        if (operation == '/' && right == 0.0) {
            set_error(state, CALC_ERROR_DIVISION_BY_ZERO, position);
            return 0.0;
        }
        value = checked_result(state, operation == '*' ? value * right : value / right, position);
    }
    return value;
}

static double parse_expression(parser *state, unsigned int depth) {
    double value = parse_term(state, depth);
    while (state->status == CALC_OK) {
        skip_whitespace(state);
        if (*state->cursor != '+' && *state->cursor != '-') {
            break;
        }

        const char operation = *state->cursor;
        const char *position = state->cursor;
        ++state->cursor;
        double right = parse_term(state, depth);
        if (state->status != CALC_OK) {
            return 0.0;
        }
        value = checked_result(state, operation == '+' ? value + right : value - right, position);
    }
    return value;
}

calc_result calc_evaluate(const char *expression) {
    if (expression == NULL) {
        return (calc_result){CALC_ERROR_INVALID_ARGUMENT, 0.0, 0U};
    }

    parser state = {expression, expression, CALC_OK, 0U};
    double value = parse_expression(&state, 0U);
    if (state.status == CALC_OK) {
        skip_whitespace(&state);
        if (*state.cursor != '\0') {
            set_error(&state, CALC_ERROR_SYNTAX, state.cursor);
        }
    }

    if (state.status != CALC_OK) {
        value = 0.0;
    }
    return (calc_result){state.status, value, state.error_offset};
}

const char *calc_status_message(calc_status status) {
    switch (status) {
    case CALC_OK:
        return "success";
    case CALC_ERROR_INVALID_ARGUMENT:
        return "invalid argument";
    case CALC_ERROR_SYNTAX:
        return "syntax error";
    case CALC_ERROR_DIVISION_BY_ZERO:
        return "division by zero";
    case CALC_ERROR_RANGE:
        return "numeric range error";
    default:
        return "unknown error";
    }
}
