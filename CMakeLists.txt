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

file(GLOB_RECURSE SCINTILLA_SRC
    "${CMAKE_CURRENT_LIST_DIR}/scintilla/src/*.cxx"
    "${CMAKE_CURRENT_LIST_DIR}/scintilla/lexlib/*.cxx"
)

file(GLOB_RECURSE SCILEXERS_SRC "${CMAKE_CURRENT_LIST_DIR}/scintilla/lexers/*.cxx")

add_library(scintilla STATIC ${SCINTILLA_SRC})
add_library(scilexers STATIC ${SCILEXERS_SRC})

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

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    set_target_properties(scintilla PROPERTIES UNITY_BUILD ON)
    target_precompile_headers(scilexers PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/ScilexersPCH.h"
    )
endif()

# Address cyclic dependencies of static libraries

target_link_libraries(scintilla scilexers)
target_link_libraries(scilexers scintilla)

# Target 'turbo'

file(GLOB_RECURSE TURBO_SRC "${CMAKE_CURRENT_LIST_DIR}/src/*.cc")
set(TURBO_MAIN "${CMAKE_CURRENT_LIST_DIR}/src/main.cc")
list(REMOVE_ITEM TURBO_SRC ${TURBO_MAIN})

add_library(${PROJECT_NAME}-core ${TURBO_SRC})

add_subdirectory(deps/fmt)
add_subdirectory(deps/libclipboard)
add_subdirectory(deps/tvision)

target_include_directories(${PROJECT_NAME}-core PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}/src"
    "${CMAKE_CURRENT_LIST_DIR}/src/platform"
    "${CMAKE_CURRENT_LIST_DIR}/deps/libclipboard/include" # libclipboard.h
    "${CMAKE_BINARY_DIR}/deps/libclipboard/include" # libclipboard-config.h
)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    set_target_properties(${PROJECT_NAME}-core PROPERTIES UNITY_BUILD ON)
    target_precompile_headers(${PROJECT_NAME}-core PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/TurboPCH.h"
    )
endif()

target_link_libraries(${PROJECT_NAME}-core PUBLIC
    scintilla
    tvision
    fmt
    clipboard
)

# Other optional dependencies
find_library(MAGIC magic)
if (MAGIC)
    target_link_libraries(${PROJECT_NAME}-core PRIVATE ${MAGIC})
    target_compile_definitions(${PROJECT_NAME}-core PRIVATE HAVE_MAGIC)
endif()

add_executable(${PROJECT_NAME} ${TURBO_MAIN})
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PROJECT_NAME}-core
)
