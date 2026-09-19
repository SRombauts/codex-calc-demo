#include "calc.h"
#include "unity.h"

#include <stddef.h>
#include <string.h>

void setUp(void) {}

void tearDown(void) {}

static void assert_value(const char *expression, double expected) {
    calc_result result = calc_evaluate(expression);
    TEST_ASSERT_EQUAL_INT(CALC_OK, result.status);
    TEST_ASSERT_DOUBLE_WITHIN(1e-12, expected, result.value);
}

static void assert_error(const char *expression, calc_status expected_status) {
    calc_result result = calc_evaluate(expression);
    TEST_ASSERT_EQUAL_INT(expected_status, result.status);
}

static void test_operator_precedence(void) {
    assert_value("2 + 3 * 4", 14.0);
    assert_value("18 / 3 - 2", 4.0);
    assert_value("10 - 2 * 3 + 1", 5.0);
}

static void test_parentheses(void) {
    assert_value("(2 + 3) * 4", 20.0);
    assert_value("2 * (3 + (4 - 1))", 12.0);
}

static void test_unary_signs(void) {
    assert_value("-12.5 / (2 + 3)", -2.5);
    assert_value("--2 + +-3", -1.0);
    assert_value("-(2 + 3) * +4", -20.0);
}

static void test_whitespace_and_decimals(void) {
    assert_value("\t .5 + 1.25 \n", 1.75);
    assert_value("2. + 1e2", 102.0);
    assert_value("1E-2 * 5", 0.05);
}

static void test_left_associativity(void) {
    assert_value("20 / 5 / 2", 2.0);
    assert_value("10 - 3 - 2", 5.0);
}

static void test_syntax_errors(void) {
    assert_error("", CALC_ERROR_SYNTAX);
    assert_error("   ", CALC_ERROR_SYNTAX);
    assert_error("1 +", CALC_ERROR_SYNTAX);
    assert_error("1 *", CALC_ERROR_SYNTAX);
    assert_error("* 2", CALC_ERROR_SYNTAX);
    assert_error("()", CALC_ERROR_SYNTAX);
    assert_error("(1 + 2", CALC_ERROR_SYNTAX);
    assert_error("1 + 2)", CALC_ERROR_SYNTAX);
    assert_error("1 2", CALC_ERROR_SYNTAX);
    assert_error("hello", CALC_ERROR_SYNTAX);
    assert_error("1e+", CALC_ERROR_SYNTAX);
    assert_error("0x1p2", CALC_ERROR_SYNTAX);
    TEST_ASSERT_EQUAL_UINT64(6U, calc_evaluate("(1 + 2x").error_offset);
}

static void test_error_offsets(void) {
    TEST_ASSERT_EQUAL_UINT64(4U, calc_evaluate("1 + * 2").error_offset);
    TEST_ASSERT_EQUAL_UINT64(6U, calc_evaluate("(1 + 2").error_offset);
    TEST_ASSERT_EQUAL_UINT64(2U, calc_evaluate("1 / 0").error_offset);
}

static void test_division_by_zero(void) {
    assert_error("1 / 0", CALC_ERROR_DIVISION_BY_ZERO);
    assert_error("1 / -0", CALC_ERROR_DIVISION_BY_ZERO);
    assert_error("1 / (2 - 2)", CALC_ERROR_DIVISION_BY_ZERO);
}

static void test_range_errors(void) {
    assert_error("1e9999", CALC_ERROR_RANGE);
    assert_error("1e99999", CALC_ERROR_RANGE);
    assert_error("1e308 * 1e308", CALC_ERROR_RANGE);
    assert_error("1e-9999", CALC_ERROR_RANGE);
    assert_error("2.2250738585072014e-308 * 2.2250738585072014e-308", CALC_ERROR_RANGE);
    assert_error("2.2250738585072014e-308 / 1e308", CALC_ERROR_RANGE);
}

static void test_invalid_argument_and_messages(void) {
    TEST_ASSERT_EQUAL_INT(CALC_ERROR_INVALID_ARGUMENT, calc_evaluate(NULL).status);
    TEST_ASSERT_EQUAL_STRING("success", calc_status_message(CALC_OK));
    TEST_ASSERT_EQUAL_STRING("invalid argument", calc_status_message(CALC_ERROR_INVALID_ARGUMENT));
    TEST_ASSERT_EQUAL_STRING("syntax error", calc_status_message(CALC_ERROR_SYNTAX));
    TEST_ASSERT_EQUAL_STRING("division by zero", calc_status_message(CALC_ERROR_DIVISION_BY_ZERO));
    TEST_ASSERT_EQUAL_STRING("numeric range error", calc_status_message(CALC_ERROR_RANGE));
    TEST_ASSERT_EQUAL_STRING("unknown error", calc_status_message((calc_status)99));
}

static void test_depth_limit(void) {
    char expression[140];
    memset(expression, '-', sizeof(expression) - 2U);
    expression[sizeof(expression) - 2U] = '1';
    expression[sizeof(expression) - 1U] = '\0';
    assert_error(expression, CALC_ERROR_SYNTAX);
}

static void test_parenthesis_depth_limit(void) {
    char expression[(2U * 129U) + 2U];
    size_t index = 0U;

    for (size_t level = 0U; level < 128U; ++level) {
        expression[index++] = '(';
    }
    expression[index++] = '1';
    for (size_t level = 0U; level < 128U; ++level) {
        expression[index++] = ')';
    }
    expression[index] = '\0';
    assert_value(expression, 1.0);

    index = 0U;
    for (size_t level = 0U; level < 129U; ++level) {
        expression[index++] = '(';
    }
    expression[index++] = '1';
    for (size_t level = 0U; level < 129U; ++level) {
        expression[index++] = ')';
    }
    expression[index] = '\0';
    calc_result result = calc_evaluate(expression);
    TEST_ASSERT_EQUAL_INT(CALC_ERROR_SYNTAX, result.status);
    TEST_ASSERT_EQUAL_UINT64(129U, result.error_offset);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_operator_precedence);
    RUN_TEST(test_parentheses);
    RUN_TEST(test_unary_signs);
    RUN_TEST(test_whitespace_and_decimals);
    RUN_TEST(test_left_associativity);
    RUN_TEST(test_syntax_errors);
    RUN_TEST(test_error_offsets);
    RUN_TEST(test_division_by_zero);
    RUN_TEST(test_range_errors);
    RUN_TEST(test_invalid_argument_and_messages);
    RUN_TEST(test_depth_limit);
    RUN_TEST(test_parenthesis_depth_limit);
    return UNITY_END();
}
