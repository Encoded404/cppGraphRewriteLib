# Add the test executable
add_executable(GraphLibraryTests simple_tests.cpp)

# Find GTest installed via vcpkg and link it along with the main library
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)
target_link_libraries(GraphLibraryTests PRIVATE
    GraphLibrary::GraphLibrary
    GTest::gtest
    GTest::gtest_main
)

# Set compiler options for the test executable
target_compile_options(GraphLibraryTests PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

# Multi-configuration generator options
if(CMAKE_CONFIGURATION_TYPES)
	target_compile_options(GraphLibraryTests PRIVATE
        "$<$<CONFIG:Debug>:-g>"
        "$<$<CONFIG:Debug>:-O0>"
        "$<$<CONFIG:Release>:-O3>"
    )
endif()

target_include_directories(GraphLibraryTests PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

# Let CTest discover individual GoogleTest tests in the binary
gtest_discover_tests(GraphLibraryTests
    PROPERTIES
        ENVIRONMENT "TEST_LOG_DIR=${CMAKE_BINARY_DIR}/Testing/logs"
)
