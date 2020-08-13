cmake_minimum_required (VERSION 3.5)
project (turbo)

# Target 'scintilla'

add_library(scintilla STATIC)

target_compile_options(scintilla PRIVATE
    -Wall
    -O3 --std=c++17
)

target_compile_definitions(scintilla PRIVATE
    CURSES
    SCI_LEXER
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

# Target 'turbo'

add_executable(turbo)

target_compile_options(turbo PRIVATE
    -Wall -Wno-missing-braces -Wno-unknown-pragmas -Wno-unused-function -Wno-pragmas -Wno-reorder
    -O2 --std=c++17 -fpermissive
)

find_path(TVISION_INCLUDE "tvision/tv.h" HINTS "${CMAKE_CURRENT_SOURCE_DIR}/tvision/include")

target_include_directories(turbo PRIVATE
    ${SCINTILLA_INC}
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/platform"
    "${TVISION_INCLUDE}"
    "${TVISION_INCLUDE}/override"
)

target_sources(turbo PRIVATE
    "src/app.cc"
    "src/main.cc"
    "src/doctree.cc"
    "src/docview.cc"
    "src/editstates.cc"
    "src/editwindow.cc"
    "src/listviews.cc"
    "src/search.cc"
    "src/styles.cc"
    "src/tscintilla.cc"
    "src/util.cc"
    "src/widgets.cc"
    "src/platform/font.cc"
    "src/platform/library.cc"
    "src/platform/listbox.cc"
    "src/platform/menu.cc"
    "src/platform/platform.cc"
    "src/platform/surface.cc"
    "src/platform/window.cc"
)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    set_target_properties(turbo PROPERTIES UNITY_BUILD ON)
    target_precompile_headers(turbo PRIVATE "include/ScintillaHeaders.h")
endif()

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
endif()

# Other optional dependencies
find_library(MAGIC magic)
if (MAGIC)
    list(APPEND LIBS ${MAGIC})
    target_compile_definitions(turbo PRIVATE HAVE_MAGIC)
endif()

target_link_libraries(turbo ${LIBS})

