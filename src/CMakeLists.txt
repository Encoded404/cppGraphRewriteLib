# Header-only logging interface (optional for consumers)
add_library(logiface INTERFACE)
target_compile_features(logiface INTERFACE cxx_std_20)
target_include_directories(logiface INTERFACE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include/modern_cmake_library>
)

add_subdirectory(lib1)