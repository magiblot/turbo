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

FILE(GLOB_RECURSE tmp LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/**")
SET(SCINTILLA_INC "")
FOREACH(entry ${tmp})
    IF(IS_DIRECTORY ${entry})
        LIST(APPEND SCINTILLA_INC ${entry})
    ENDIF()
ENDFOREACH()

target_include_directories(scintilla PRIVATE
    ${SCINTILLA_INC}
)

file(GLOB_RECURSE SCINTILLA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/scintilla/**/*.cxx")
target_sources(scintilla PRIVATE ${SCINTILLA_SRC})

# Target 'tvedit'

add_executable(tvedit)

target_compile_options(tvedit PRIVATE
    -Wall -Wno-missing-braces -Wno-unknown-pragmas -Wno-unused-function -Wno-pragmas -Wno-reorder
    -O0 -g --std=c++17 -fpermissive
)

find_path(TVISION_INCLUDE "tvision/tv.h" HINTS "${CMAKE_CURRENT_SOURCE_DIR}/tvision/include")

target_include_directories(tvedit PRIVATE
    ${SCINTILLA_INC}
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/platform"
    "${TVISION_INCLUDE}"
    "${TVISION_INCLUDE}/override"
)

target_sources(tvedit PRIVATE
    "src/app.cc"
    "src/main.cc"
    "src/docview.cc"
    "src/drawviews.cc"
    "src/editwindow.cc"
    "src/tscintilla.cc"
    "src/util.cc"
    "src/widgets.cc"
    "src/platform/font.cc"
    "src/platform/listbox.cc"
    "src/platform/menu.cc"
    "src/platform/platform.cc"
    "src/platform/surface.cc"
    "src/platform/window.cc"
)

set_target_properties(tvedit PROPERTIES UNITY_BUILD ON)
target_precompile_headers(tvedit PRIVATE "include/ScintillaHeaders.h")

# Dependency 'tvision'
# Requires building manually if not installed.

find_library(LIBTVISION tvision HINTS "${CMAKE_CURRENT_SOURCE_DIR}/tvision")

# Other dependencies
find_library(LIBNCURSESW ncursesw)
find_library(FMT fmt)

set(LIBS scintilla ${LIBTVISION} ${FMT} ${LIBNCURSESW})

# Optional dependencies, compulsory if found when building 'tvision'
find_library(GPM gpm)
if (GPM)
    list(APPEND LIBS ${GPM})
    string(APPEND CMAKE_CXX_FLAGS " -DHAVE_GPM")
endif()

target_link_libraries(tvedit ${LIBS})
