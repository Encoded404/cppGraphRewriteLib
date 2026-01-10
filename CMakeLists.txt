cmake_minimum_required(VERSION 3.30)
# technically the only reason for this high of a cmake version is the policy for boost, it could be set some other way but i couldnt bother

project(cppGraphLibrary VERSION 1.0.0 LANGUAGES CXX)

include(CTest)

# get boost graph library via vcpkg
find_package(Boost REQUIRED COMPONENTS graph)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(CLANG_TIDY_ENABLED "Enable Clang-Tidy static analysis" ON)
option(ENABLE_LOGGING "Enable header-only logging interface" ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# disable Clang-Tidy if not using Clang or GCC
if(CLANG_TIDY_ENABLED AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	set(CLANG_TIDY_ENABLED OFF)
endif()
if(CLANG_TIDY_ENABLED)
	include(ConfigureClangTidy)
endif()

add_subdirectory(src)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()