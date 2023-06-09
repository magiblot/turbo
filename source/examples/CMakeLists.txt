# This file can be built either individually or as part of Turbo's
# build hierarchy.

cmake_minimum_required(VERSION 3.5)
project(turbo-examples)

set(MASTER_PROJECT FALSE)
if (CMAKE_CURRENT_LIST_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MASTER_PROJECT TRUE)
endif()

if (MASTER_PROJECT)
    # We have to import these when building this file standalone.
    find_package(tvision CONFIG REQUIRED)
    find_package(turbo CONFIG REQUIRED)
    add_library(turbo-core ALIAS turbo::turbo-core)
endif()

if (TURBO_BUILD_EXAMPLES OR MASTER_PROJECT)
    function(add_example_generic app)
        add_executable(${app} ${app}.cc)
        target_link_libraries(${app} PRIVATE turbo-core)
    endfunction()

    add_example_generic(demo-with-custom-editor-window)
endif()
