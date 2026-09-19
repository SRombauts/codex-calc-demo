cmake_minimum_required(VERSION 3.20)

project(calc VERSION 0.1.0 LANGUAGES C)

include(CTest)
include(GNUInstallDirs)
include(cmake/ProjectOptions.cmake)

add_executable(calc src/bootstrap.c)
calc_apply_project_options(calc)

install(TARGETS calc RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(BUILD_TESTING)
    add_executable(bootstrap_test tests/bootstrap_test.c)
    calc_apply_project_options(bootstrap_test)
    add_test(NAME bootstrap COMMAND bootstrap_test)
    add_test(NAME cli_bootstrap COMMAND calc)
endif()

set(CPACK_PACKAGE_VENDOR "calc contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ISO C17 command-line expression calculator")
if(NOT CPACK_PACKAGE_FILE_NAME)
    set(CPACK_PACKAGE_FILE_NAME "calc-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
endif()
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)

