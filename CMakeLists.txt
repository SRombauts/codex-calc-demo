cmake_minimum_required(VERSION 3.20)

project(calc VERSION 0.1.0 LANGUAGES C)

include(CTest)
include(GNUInstallDirs)
include(cmake/ProjectOptions.cmake)

add_library(calc_core STATIC src/calc.c)
target_include_directories(calc_core PUBLIC include)
calc_apply_project_options(calc_core)

add_executable(calc src/main.c)
target_link_libraries(calc PRIVATE calc_core)
calc_apply_project_options(calc)

install(TARGETS calc RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES README.md LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/calc)

if(BUILD_TESTING)
    add_library(unity STATIC tests/unity/unity.c)
    target_include_directories(unity PUBLIC tests/unity)
    target_compile_definitions(unity PUBLIC UNITY_INCLUDE_DOUBLE)
    set_target_properties(unity PROPERTIES C_STANDARD 17 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)

    add_executable(calc_unit_tests tests/test_calc.c)
    target_link_libraries(calc_unit_tests PRIVATE calc_core unity)
    calc_apply_project_options(calc_unit_tests)
    add_test(NAME unit COMMAND calc_unit_tests)
    add_test(
        NAME cli
        COMMAND ${CMAKE_COMMAND} -DCALC_EXECUTABLE=$<TARGET_FILE:calc> -P
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_tests.cmake
    )
endif()

set(CPACK_PACKAGE_VENDOR "calc contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ISO C17 command-line expression calculator")
if(NOT CPACK_PACKAGE_FILE_NAME)
    set(CPACK_PACKAGE_FILE_NAME "calc-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
endif()
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)
