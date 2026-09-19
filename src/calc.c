#include "calc.h"

#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#define CALC_DECIMAL_DIGITS 18U

enum { CALC_MAX_DEPTH = 128, CALC_MAX_DECIMAL_EXPONENT = 10000 };

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

static double checked_result(parser *state, double value, const char *position,
                             int zero_is_underflow) {
    if (!isfinite(value) || (zero_is_underflow != 0 && value == 0.0)) {
        set_error(state, CALC_ERROR_RANGE, position);
        return 0.0;
    }
    return value;
}

static void collect_digit(unsigned int digit, uint64_t *mantissa, unsigned int *kept_digits,
                          size_t *significant_digits) {
    if (digit != 0U || *significant_digits != 0U) {
        ++*significant_digits;
        if (*kept_digits < CALC_DECIMAL_DIGITS) {
            *mantissa = (*mantissa * 10U) + digit;
            ++*kept_digits;
        }
    }
}

static int decimal_scale(size_t fractional_digits, size_t dropped_digits, int explicit_exponent) {
    int scale = explicit_exponent;
    if (dropped_digits >= fractional_digits) {
        size_t difference = dropped_digits - fractional_digits;
        if (difference > (size_t)(CALC_MAX_DECIMAL_EXPONENT - scale)) {
            return CALC_MAX_DECIMAL_EXPONENT;
        }
        scale += (int)difference;
    } else {
        size_t difference = fractional_digits - dropped_digits;
        if (difference > (size_t)(CALC_MAX_DECIMAL_EXPONENT + scale)) {
            return -CALC_MAX_DECIMAL_EXPONENT;
        }
        scale -= (int)difference;
    }
    return scale;
}

static long double apply_decimal_scale(long double value, int exponent) {
    unsigned int power = (unsigned int)(exponent < 0 ? -exponent : exponent);
    long double factor = exponent < 0 ? 0.1L : 10.0L;
    while (power != 0U) {
        if ((power & 1U) != 0U) {
            value *= factor;
        }
        factor *= factor;
        power >>= 1U;
    }
    return value;
}

static int parse_explicit_exponent(const char **cursor) {
    const char *exponent_start = *cursor;
    const char *position = exponent_start + 1;
    int sign = 1;
    int exponent = 0;

    if (*position == '+' || *position == '-') {
        if (*position == '-') {
            sign = -1;
        }
        ++position;
    }
    if (isdigit((unsigned char)*position) == 0) {
        return 0;
    }
    while (isdigit((unsigned char)*position) != 0) {
        unsigned int digit = (unsigned int)(*position - '0');
        if (exponent < CALC_MAX_DECIMAL_EXPONENT) {
            exponent = (exponent * 10) + (int)digit;
            if (exponent > CALC_MAX_DECIMAL_EXPONENT) {
                exponent = CALC_MAX_DECIMAL_EXPONENT;
            }
        }
        ++position;
    }
    *cursor = position;
    return sign * exponent;
}

static double parse_number(parser *state) {
    const char *start = state->cursor;
    const char *position = start;
    uint64_t mantissa = 0U;
    unsigned int kept_digits = 0U;
    size_t significant_digits = 0U;
    size_t fractional_digits = 0U;

    if (isdigit((unsigned char)*position) == 0 &&
        !(*position == '.' && isdigit((unsigned char)position[1]) != 0)) {
        set_error(state, CALC_ERROR_SYNTAX, position);
        return 0.0;
    }
    while (isdigit((unsigned char)*position) != 0) {
        collect_digit((unsigned int)(*position - '0'), &mantissa, &kept_digits,
                      &significant_digits);
        ++position;
    }
    if (*position == '.') {
        ++position;
        while (isdigit((unsigned char)*position) != 0) {
            collect_digit((unsigned int)(*position - '0'), &mantissa, &kept_digits,
                          &significant_digits);
            ++fractional_digits;
            ++position;
        }
    }

    int explicit_exponent = 0;
    if (*position == 'e' || *position == 'E') {
        const char *exponent_start = position;
        explicit_exponent = parse_explicit_exponent(&position);
        if (position == exponent_start) {
            state->cursor = exponent_start;
            return (double)mantissa;
        }
    }
    state->cursor = position;
    if (mantissa == 0U) {
        return 0.0;
    }

    size_t dropped_digits = significant_digits - (size_t)kept_digits;
    int scale = decimal_scale(fractional_digits, dropped_digits, explicit_exponent);
    long double scaled = apply_decimal_scale((long double)mantissa, scale);
    double value = (double)scaled;
    if (!isfinite(scaled) || scaled == 0.0L || !isfinite(value) || value == 0.0) {
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
        set_error(state, CALC_ERROR_SYNTAX, state->cursor);
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
    return checked_result(state, -value, position, 0);
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
        int zero_is_underflow = value != 0.0 && right != 0.0;
        value = checked_result(state, operation == '*' ? value * right : value / right, position,
                               zero_is_underflow);
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
        value =
            checked_result(state, operation == '+' ? value + right : value - right, position, 0);
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
