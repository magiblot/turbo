# 'find_package' exports information through environment variables.
# So, for this to work properly, this file has to be imported with 'include'
# rather than 'add_subdirectory'.

if (TURBO_USE_SYSTEM_DEPS)
    # fmt
    find_package(fmt CONFIG REQUIRED)
    add_library(fmt ALIAS fmt::fmt)
    get_target_property(_FMT fmt LOCATION)
    message(STATUS "Found 'fmt': ${_FMT}")

    # libclipboard
    find_library(CLIPBOARD clipboard)
    if (NOT CLIPBOARD)
        message(FATAL_ERROR "Library 'libclipboard' not found")
    else()
        message(STATUS "Found 'libclipboard': ${CLIPBOARD}")
    endif()
    find_path(CLIPBOARD_INCLUDE "libclipboard.h")
    if (NOT CLIPBOARD_INCLUDE)
        message(FATAL_ERROR "'libclipboard' development headers not found")
    endif()

    add_library(clipboard UNKNOWN IMPORTED)
    set_target_properties(clipboard PROPERTIES
        IMPORTED_LOCATION ${CLIPBOARD}
        INTERFACE_INCLUDE_DIRECTORIES ${CLIPBOARD_INCLUDE}
    )
    if (NOT WIN32)
        foreach (lib pthread xcb)
            find_library(_${lib} ${lib})
            if (_${lib})
                message(STATUS "Found 'libclipboard' dependency '${lib}': ${_${lib}}")
                target_link_libraries(clipboard INTERFACE ${_${lib}})
            endif()
        endforeach()
    endif()
else()
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/fmt" EXCLUDE_FROM_ALL)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/libclipboard" EXCLUDE_FROM_ALL)
endif()

if (TURBO_USE_SYSTEM_TVISION)
    find_package(tvision CONFIG REQUIRED)
    add_library(tvision ALIAS tvision::tvision)
    get_target_property(_TVISION tvision LOCATION)
    message(STATUS "Found 'tvision': ${_TVISION}")
else()
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/tvision" EXCLUDE_FROM_ALL)
endif()
