# Main library target
add_library(cppGraphLibrary STATIC)
add_library(CppGraphLibrary::GraphLibrary ALIAS cppGraphLibrary)

target_compile_features(cppGraphLibrary PUBLIC cxx_std_20)
target_link_libraries(cppGraphLibrary PUBLIC logiface)

target_link_libraries(cppGraphLibrary PRIVATE Boost::graph)

target_compile_definitions(cppGraphLibrary
    PUBLIC
        $<$<BOOL:${ENABLE_LOGGING}>:LOGIFACE_ENABLE_LOGGING=1>
        $<$<NOT:$<BOOL:${ENABLE_LOGGING}>>:LOGIFACE_ENABLE_LOGGING=0>
)

target_compile_options(cppGraphLibrary PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Werror>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

if(CMAKE_CONFIGURATION_TYPES)
    target_compile_options(cppGraphLibrary PRIVATE
                "$<$<CONFIG:Debug>:-g>"
                "$<$<CONFIG:Debug>:-O0>"
                "$<$<CONFIG:Release>:-O3>"
        )
endif()

target_include_directories(cppGraphLibrary
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

target_sources(cppGraphLibrary PRIVATE
    Graph.cpp
)
