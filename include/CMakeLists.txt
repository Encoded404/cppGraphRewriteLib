# CMake helpers for the header-only GraphLibrary

# Create an INTERFACE (header-only) target named GraphLibrary
add_library(GraphLibrary INTERFACE)

target_include_directories(GraphLibrary INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external/cppLoggingInterface/include>
)

# Provide an alias for nicer namespacing
if(NOT TARGET GraphLibrary::GraphLibrary)
  add_library(GraphLibrary::GraphLibrary ALIAS GraphLibrary)
endif()

# make anyone consuming the library link to boost graph as well
find_package(Boost REQUIRED COMPONENTS graph CONFIG)

target_link_libraries(GraphLibrary INTERFACE Boost::graph)

# Install headers and export target for downstream use
install(TARGETS GraphLibrary
  EXPORT GraphLibraryTargets
  INCLUDES DESTINATION include
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/GraphLibrary
  DESTINATION include
)

install(EXPORT GraphLibraryTargets
  FILE GraphLibraryTargets.cmake
  NAMESPACE GraphLibrary::
  DESTINATION lib/cmake/GraphLibrary
)
