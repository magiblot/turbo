cmake_minimum_required (VERSION 3.5)

option(TURBO_BUILD_APP "Build main editor application" ON)
option(TURBO_BUILD_EXAMPLES "Build example apps" OFF)
option(TURBO_USE_SYSTEM_DEPS "Use system-wide dependencies instead of submodules (except Turbo Vision)" OFF)
option(TURBO_USE_SYSTEM_TVISION "Use system-wide Turbo Vision instead of the submodule" OFF)
option(TURBO_USE_STATIC_RTL "Link against the static version of the runtime library (MSVC only)" OFF)
option(TURBO_OPTIMIZE_BUILD "Use Precompiled Headers and Unity Build for the core library" ON)

if (TURBO_USE_SYSTEM_DEPS AND TURBO_USE_SYSTEM_TVISION)
    set(MAY_INSTALL_LIB TRUE)
endif()

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.15.0")
    cmake_policy(SET CMP0091 NEW)
    if (TURBO_USE_STATIC_RTL)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

project (turbo)

# Target 'scintilla'

# These two could be built in one single target, but we want to enable Unity Build
# for 'scintilla' and precompiled headers for 'scilexers'.

file(GLOB_RECURSE SCINTILLA_SRC
    "${CMAKE_CURRENT_LIST_DIR}/source/scintilla/src/*.cxx"
    "${CMAKE_CURRENT_LIST_DIR}/source/scintilla/lexlib/*.cxx"
)

list(APPEND SCILEXERS_SRC
    "${CMAKE_CURRENT_LIST_DIR}/source/scintilla/lexers/LexAsm.cxx"
    "${CMAKE_CURRENT_LIST_DIR}/source/scintilla/lexers/LexBash.cxx"
    "${CMAKE_CURRENT_LIST_DIR}/source/scintilla/lexers/LexCPP.cxx"
    "${CMAKE_CURRENT_LIST_DIR}/source/scintilla/lexers/LexMake.cxx"
    "${CMAKE_CURRENT_LIST_DIR}/source/scintilla/lexers/LexPython.cxx"
    "${CMAKE_CURRENT_LIST_DIR}/source/scintilla/lexers/LexRuby.cxx"
    "${CMAKE_CURRENT_LIST_DIR}/source/scintilla/lexers/LexRust.cxx"
)

add_library(scintilla OBJECT ${SCINTILLA_SRC})
add_library(scilexers OBJECT ${SCILEXERS_SRC})

function(turbo_set_warnings t)
    if (NOT ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC"))
        target_compile_options(${t} PRIVATE
            -Wall
        )
    else()
        target_compile_options(${t} PRIVATE
            /wd4244 # Possible data loss in type conversion
        )
    endif()
endfunction()

foreach(t scintilla scilexers)
    target_compile_features(${t} PUBLIC cxx_std_17)
    turbo_set_warnings(${t})
    target_include_directories(${t} PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/turbo/scintilla/include"
        "${CMAKE_CURRENT_LIST_DIR}/include/turbo/scintilla/lexlib"
        "${CMAKE_CURRENT_LIST_DIR}/include/turbo/scintilla/src"
    )
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_compile_options(${t} PRIVATE
            /wd4514 /wd4166 /wd4068 /wd4996 /wd4250 /wd4267
            /permissive- /Zc:__cplusplus
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

# Dependencies

include(deps/CMakeLists.txt)

# Target 'turbo-core'

set(TURBO turbo)

file(GLOB_RECURSE TURBO_CORE_SRC "${CMAKE_CURRENT_LIST_DIR}/source/${TURBO}-core/*.cc")
add_library(${TURBO}-core
    ${TURBO_CORE_SRC}
    $<TARGET_OBJECTS:scintilla>
    $<TARGET_OBJECTS:scilexers>
)

target_compile_features(${TURBO}-core PRIVATE cxx_std_17)
turbo_set_warnings(${TURBO}-core)
target_include_directories(${TURBO}-core PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
)
target_include_directories(${TURBO}-core PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/source/${TURBO}-core"
    "${CMAKE_CURRENT_LIST_DIR}/source/${TURBO}-core/platform"
    "${CMAKE_CURRENT_LIST_DIR}/deps/libclipboard/include" # libclipboard.h
    "${CMAKE_BINARY_DIR}/deps/libclipboard/include" # libclipboard-config.h
)
target_link_libraries(${TURBO}-core PUBLIC
    tvision
    fmt
    clipboard
)
if (MAY_INSTALL_LIB)
    install(TARGETS fmt clipboard EXPORT ${TURBO}-config)
    install(TARGETS ${TURBO}-core
        EXPORT ${TURBO}-config
        ARCHIVE DESTINATION lib
        COMPONENT library
    )
    install(EXPORT ${TURBO}-config
        DESTINATION lib/cmake/${TURBO}
        NAMESPACE ${TURBO}::
        FILE ${TURBO}-config.cmake
        COMPONENT library
    )
    install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/turbo" DESTINATION include)
endif()

# Optional dependencies

find_library(MAGIC magic)
if (MAGIC)
    target_link_libraries(${TURBO}-core PRIVATE ${MAGIC})
    target_compile_definitions(${TURBO}-core PRIVATE HAVE_MAGIC)
endif()

# Target 'turbo'

if (TURBO_BUILD_APP)
    file(GLOB_RECURSE TURBO_SRC "${CMAKE_CURRENT_LIST_DIR}/source/${TURBO}/*.cc")
    add_executable(${TURBO} ${TURBO_SRC})
    target_compile_features(${TURBO} PRIVATE cxx_std_17)
    turbo_set_warnings(${TURBO})
    target_link_libraries(${TURBO} PRIVATE
        ${TURBO}-core
    )
    install(TARGETS ${TURBO} RUNTIME DESTINATION bin)
endif()

# Examples

add_subdirectory(source/examples)

# Optional build optimization

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0" AND TURBO_OPTIMIZE_BUILD)
    set_target_properties(${TURBO}-core PROPERTIES UNITY_BUILD ON)
    target_precompile_headers(${TURBO}-core PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/include/TurboPCH.h"
    )
    if (TURBO_BUILD_APP)
        set_target_properties(${TURBO} PROPERTIES UNITY_BUILD ON)
    endif()

endif()
