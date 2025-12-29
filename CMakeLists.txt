cmake_minimum_required(VERSION 3.15)

project(ModernCMakeLibraryTemplate VERSION 1.0.0 LANGUAGES CXX)

include(CTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(CLANG_TIDY_ENABLED "Enable Clang-Tidy static analysis" ON)
option(ENABLE_LOGGING "Enable header-only logging interface" ON)

set(mclt_export_name ModernCMakeLibraryTemplateTargets)
set(mclt_config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/ModernCMakeLibraryTemplate")

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

configure_package_config_file(
	cmake/ModernCMakeLibraryTemplateConfig.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/ModernCMakeLibraryTemplateConfig.cmake"
	INSTALL_DESTINATION "${mclt_config_install_dir}"
)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/ModernCMakeLibraryTemplateConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/ModernCMakeLibraryTemplateConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/ModernCMakeLibraryTemplateConfigVersion.cmake"
	DESTINATION "${mclt_config_install_dir}"
)

install(
	EXPORT ${mclt_export_name}
	NAMESPACE ModernCMakeLibrary::
	DESTINATION "${mclt_config_install_dir}"
)