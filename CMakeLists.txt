cmake_minimum_required(VERSION 3.14)
# technically the only reason for this high of a cmake version is the policy for boost, it could be set some other way but i couldnt bother
# nvm when this is a subdir it has to set the policy anyway to avoid warnings
if(CMAKE_VERSION VERSION_LESS "3.30.0" AND POLICY CMP0074)
	message("CMake version is ${CMAKE_VERSION} which is less than 3.30.0, setting CMP0074 to NEW")
	cmake_policy(SET CMP0074 NEW)
endif()

# detect if this directory is the top-level project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(IS_TOP_PROJECT ON)
else()
    set(IS_TOP_PROJECT OFF)
endif()

project(cppGraphLibrary VERSION 1.0.0 LANGUAGES CXX)

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    message(WARNING "vcpkg not detected; dependencies may fail")
endif()

include(CTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(CLANG_TIDY_ENABLED "Enable Clang-Tidy static analysis" ON)
option(ENABLE_LOGGING "Enable header-only logging interface" ON)

option(COMPILE_EXAMPLES "compile example binaries" ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# disable Clang-Tidy if not using Clang or GCC
if(IS_TOP_PROJECT)
	if(CLANG_TIDY_ENABLED AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set(CLANG_TIDY_ENABLED OFF)
		message(WARNING "Clang-Tidy disabled: not using Clang compiler")
	endif()
	if(CLANG_TIDY_ENABLED)
		include(ConfigureClangTidy)
	endif()
endif()

add_subdirectory(include)

if(IS_TOP_PROJECT)
	if(BUILD_TESTING)
		add_subdirectory(tests)
	endif()

	if(COMPILE_EXAMPLES)
		add_subdirectory(examples)
	endif()
endif()