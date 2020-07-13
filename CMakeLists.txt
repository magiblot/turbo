cmake_minimum_required (VERSION 3.5)
project (tvedit)

# Target 'scintilla'

add_library(scintilla STATIC)

target_compile_options(scintilla PRIVATE
    -Wall
    -O3 -g --std=c++17
)

target_compile_definitions(scintilla PRIVATE
    CURSES
)

target_include_directories(scintilla PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/lexlib"
    "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/src"
)

file(GLOB_RECURSE SCINTILLA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/**/*.cxx")
target_sources(scintilla PRIVATE ${SCINTILLA_SRC})

# Target 'tvedit'

add_executable(tvedit)

target_compile_options(tvedit PRIVATE
    -Wall -Wno-missing-braces -Wno-unknown-pragmas -Wno-unused-function -Wno-pragmas -Wno-reorder
    -O2 -g --std=c++17 -fpermissive
)

target_include_directories(tvedit
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE source/tvedit
)

file(GLOB_RECURSE TVEDIT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/source/**/*.cc")
target_sources(tvedit PRIVATE ${TVEDIT_SRC})

# Dependency 'tvision'
# Requires building manually if not installed.

find_library(LIBTVISION tvision HINTS "${CMAKE_CURRENT_SOURCE_DIR}/tvision")

# Other dependencies
find_library(LIBNCURSESW ncursesw)

set(LIBS scintilla ${LIBTVISION} ${LIBNCURSESW})

# Optional dependencies, compulsory if found when building 'tvision'
find_library(GPM gpm)
if (GPM)
    list(APPEND LIBS ${GPM})
    string(APPEND CMAKE_CXX_FLAGS " -DHAVE_GPM")
endif()

target_link_libraries(tvedit ${LIBS})
