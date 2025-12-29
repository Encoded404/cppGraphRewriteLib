# Main library target
add_library(modern_cmake_library STATIC)
add_library(ModernCMakeLibrary::modern_cmake_library ALIAS modern_cmake_library)

target_compile_features(modern_cmake_library PUBLIC cxx_std_20)
target_link_libraries(modern_cmake_library PUBLIC logiface)

target_compile_definitions(modern_cmake_library
    PUBLIC
        $<$<BOOL:${ENABLE_LOGGING}>:LOGIFACE_ENABLE_LOGGING=1>
        $<$<NOT:$<BOOL:${ENABLE_LOGGING}>>:LOGIFACE_ENABLE_LOGGING=0>
)

target_compile_options(modern_cmake_library PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Werror>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

if(CMAKE_CONFIGURATION_TYPES)
    target_compile_options(modern_cmake_library PRIVATE
                "$<$<CONFIG:Debug>:-g>"
                "$<$<CONFIG:Debug>:-O0>"
                "$<$<CONFIG:Release>:-O3>"
        )
endif()

target_include_directories(modern_cmake_library
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/modern_cmake_library>
)

target_sources(modern_cmake_library PRIVATE
        lib_example.cpp
        lib_privateCode.cpp
)

install(
    TARGETS modern_cmake_library logiface
    EXPORT ${mclt_export_name}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION include/modern_cmake_library
)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION include/modern_cmake_library
    FILES_MATCHING PATTERN "*.hpp"
)
