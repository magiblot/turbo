if (TURBO_BUILD_EXAMPLES)
    function(add_example_generic app)
        add_executable(${app} ${app}.cc)
        target_link_libraries(${app} PRIVATE ${TURBO}-core)
    endfunction()

    add_example_generic(demo)
endif()
