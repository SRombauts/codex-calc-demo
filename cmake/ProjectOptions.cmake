option(CALC_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(CALC_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(CALC_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(CALC_ENABLE_COVERAGE "Enable gcov-compatible coverage" OFF)

function(calc_apply_project_options target)
    set_target_properties(${target} PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
    )

    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
        if(CALC_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wformat=2
            -Wshadow
            -Wsign-conversion
            -Wundef
        )
        if(CALC_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()

    if(CALC_ENABLE_ASAN OR CALC_ENABLE_UBSAN)
        if(MSVC)
            message(FATAL_ERROR "Sanitizer presets require GCC or Clang")
        endif()
        set(sanitizers "")
        if(CALC_ENABLE_ASAN)
            list(APPEND sanitizers "address")
        endif()
        if(CALC_ENABLE_UBSAN)
            list(APPEND sanitizers "undefined")
        endif()
        list(JOIN sanitizers "," sanitizer_list)
        target_compile_options(${target} PRIVATE -fno-omit-frame-pointer -fsanitize=${sanitizer_list})
        target_link_options(${target} PRIVATE -fno-omit-frame-pointer -fsanitize=${sanitizer_list})
    endif()

    if(CALC_ENABLE_COVERAGE)
        if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
            message(FATAL_ERROR "Coverage requires GCC")
        endif()
        target_compile_options(${target} PRIVATE -O0 -g --coverage)
        target_link_options(${target} PRIVATE --coverage)
    endif()
endfunction()
