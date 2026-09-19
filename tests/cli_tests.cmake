if(NOT DEFINED CALC_EXECUTABLE)
    message(FATAL_ERROR "CALC_EXECUTABLE is required")
endif()

function(normalize_newlines variable_name)
    string(REPLACE "\r\n" "\n" normalized "${${variable_name}}")
    set(${variable_name} "${normalized}" PARENT_SCOPE)
endfunction()

function(assert_cli expression expected_code expected_stdout expected_stderr)
    execute_process(
        COMMAND "${CALC_EXECUTABLE}" "${expression}"
        RESULT_VARIABLE actual_code
        OUTPUT_VARIABLE actual_stdout
        ERROR_VARIABLE actual_stderr
    )
    normalize_newlines(actual_stdout)
    normalize_newlines(actual_stderr)
    if(NOT actual_code EQUAL expected_code)
        message(FATAL_ERROR "${expression}: expected exit ${expected_code}, got ${actual_code}")
    endif()
    if(NOT actual_stdout STREQUAL expected_stdout)
        message(FATAL_ERROR "${expression}: unexpected stdout '${actual_stdout}'")
    endif()
    if(NOT actual_stderr STREQUAL expected_stderr)
        message(FATAL_ERROR "${expression}: unexpected stderr '${actual_stderr}'")
    endif()
endfunction()

assert_cli("2 + 3 * 4" 0 "14\n" "")
assert_cli("(2 + 3) * 4" 0 "20\n" "")
assert_cli("-12.5 / (2 + 3)" 0 "-2.5\n" "")
assert_cli("1 +" 3 "" "calc: syntax error at byte 3\n")
assert_cli("1 / 0" 4 "" "calc: division by zero at byte 2\n")
assert_cli("1e9999" 5 "" "calc: numeric range error at byte 0\n")

execute_process(
    COMMAND "${CALC_EXECUTABLE}"
    RESULT_VARIABLE usage_code
    OUTPUT_VARIABLE usage_stdout
    ERROR_VARIABLE usage_stderr
)
normalize_newlines(usage_stdout)
normalize_newlines(usage_stderr)
if(NOT usage_code EQUAL 2 OR NOT usage_stdout STREQUAL "" OR
   NOT usage_stderr STREQUAL "usage: calc \"EXPRESSION\"\n")
    message(FATAL_ERROR "usage contract failed")
endif()
