# Header-only logging interface (optional for consumers)
add_library(logiface INTERFACE)
target_compile_features(logiface INTERFACE cxx_std_20)
target_include_directories(logiface INTERFACE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

add_subdirectory(GraphLibrary)