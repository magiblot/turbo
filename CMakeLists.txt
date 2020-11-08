cmake_minimum_required (VERSION 3.5)
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.15.0")
    cmake_policy(SET CMP0091 NEW)
    option(TURBO_USE_STATIC_RTL "Link against the static version of the runtime library (MSVC only)" OFF)
    if (TURBO_USE_STATIC_RTL)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

project (turbo)

# Target 'scintilla'

# These two could be built in one single target, but we want to enable Unity Build
# for 'scintilla' and precompiled headers for 'scilexers'.

add_library(scintilla STATIC)
add_library(scilexers STATIC)

foreach(t scintilla scilexers)
    target_compile_features(${t} PUBLIC cxx_std_17)
    target_include_directories(${t} PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/scintilla/include"
        "${CMAKE_CURRENT_LIST_DIR}/scintilla/lexlib"
        "${CMAKE_CURRENT_LIST_DIR}/scintilla/src"
    )
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_compile_options(${t} PUBLIC
            /wd4514 /wd4166 /wd4068 /wd4996 /wd4250 /wd4267
            /permissive- /Zc:__cplusplus
        )
    else()
        target_compile_options(${t} PUBLIC
            -Wall
        )
    endif()
    target_compile_definitions(${t} PRIVATE
        CURSES
        SCI_LEXER
    )
endforeach()

file(GLOB_RECURSE SCINTILLA_SRC
    "${CMAKE_CURRENT_LIST_DIR}/scintilla/src/*.cxx"
    "${CMAKE_CURRENT_LIST_DIR}/scintilla/lexlib/*.cxx"
)
target_sources(scintilla PRIVATE ${SCINTILLA_SRC})

file(GLOB_RECURSE SCILEXERS_SRC "${CMAKE_CURRENT_LIST_DIR}/scintilla/lexers/*.cxx")
target_sources(scilexers PRIVATE ${SCILEXERS_SRC})

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    set_target_properties(scintilla PROPERTIES UNITY_BUILD ON)
    target_precompile_headers(scilexers PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/Scilexers.h"
    )
endif()

# Target 'turbo'

add_executable(${PROJECT_NAME})

add_subdirectory(deps/fmt)

add_subdirectory(deps/tvision)
find_path(TVISION_INCLUDE "tvision/tv.h" HINTS "${CMAKE_CURRENT_LIST_DIR}/deps/tvision/include")

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}/src"
    "${CMAKE_CURRENT_LIST_DIR}/src/platform"
)

file(GLOB_RECURSE TURBO_SRC "${CMAKE_CURRENT_LIST_DIR}/src/*.cc")
target_sources(${PROJECT_NAME} PRIVATE ${TURBO_SRC})

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
    target_precompile_headers(${PROJECT_NAME} PRIVATE
        "include/ScintillaHeaders.h"
        "${TVISION_INCLUDE}/tvision/tv.h"
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    scintilla
    scilexers
    tvision
    fmt
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

# Other optional dependencies
find_library(MAGIC magic)
if (MAGIC)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${MAGIC})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_MAGIC)
endif()

target_link_libraries(turbo ${LIBS})

