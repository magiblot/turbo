# 'find_package' exports information through environment variables.
# So, for this to work properly, this file has to be imported with 'include'
# rather than 'add_subdirectory'.

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/fmt")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/libclipboard")

if (TURBO_USE_SYSTEM_TVISION)
    find_package(tvision CONFIG REQUIRED)
    get_property(TVISION TARGET tvision PROPERTY LOCATION)
    message(STATUS "Found 'tvision': ${TVISION}")
else()
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/tvision")
endif()
