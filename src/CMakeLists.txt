if(GRAPHLIBRARY_BUILD_MODULE)
    # Enable the logging module so import logiface; is available
    set(LOGIFACE_BUILD_MODULE ON CACHE BOOL "" FORCE)

    # If the consumer also wants import std; in the GraphLibrary module,
    # propagate the toggle to the logging module as well.
    if(GRAPHLIBRARY_USE_STD_MODULE)
        set(LOGIFACE_USE_STD_MODULE ON CACHE BOOL "" FORCE)
    endif()

    # Add the cppLoggingInterface subdirectory so the logging module target
    # (logiface / logiface_module) is available for linking.
    add_subdirectory(${PROJECT_SOURCE_DIR}/external/cppLoggingInterface
                     ${CMAKE_BINARY_DIR}/external/cppLoggingInterface)

    # ── C++ Standard split ──
    # C++23 + CMAKE_CXX_MODULE_STD is required when using import std;
    # (GRAPHLIBRARY_USE_STD_MODULE=ON).  C++20 is sufficient for C++20 modules
    # with traditional #include in the global module fragment.
    if(GRAPHLIBRARY_USE_STD_MODULE)
        set(CMAKE_CXX_STANDARD 23)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
        set(CMAKE_CXX_MODULE_STD ON)
    else()
        set(CMAKE_CXX_STANDARD 20)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
    endif()

    add_library(GraphLibrary_module STATIC)

    target_sources(GraphLibrary_module PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/GraphLibrary/GraphLibrary.cppm
    )

    target_compile_features(GraphLibrary_module PUBLIC
        $<IF:$<BOOL:${GRAPHLIBRARY_USE_STD_MODULE}>,cxx_std_23,cxx_std_20>
    )

    if(GRAPHLIBRARY_USE_STD_MODULE)
        set_target_properties(GraphLibrary_module PROPERTIES CXX_MODULE_STD ON)
    endif()

    # Pass the std module toggle as a compile definition
    target_compile_definitions(GraphLibrary_module
        PUBLIC
            $<$<BOOL:${GRAPHLIBRARY_USE_STD_MODULE}>:GRAPHLIBRARY_USE_STD_MODULE=1>
    )

    # Depend on GraphLibrary INTERFACE target to get include paths
    # (include/ and external/cppLoggingInterface/include/).
    target_link_libraries(GraphLibrary_module PUBLIC GraphLibrary logiface)

    # Provide a namespaced alias for discoverability.
    # Consumers explicitly opt in by linking to GraphLibrary::Modules
    # when they want to use import GraphLibrary; in their translation units.
    add_library(GraphLibrary::Modules ALIAS GraphLibrary_module)
endif()
