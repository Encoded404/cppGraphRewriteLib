add_executable(simpleExample main.cpp)

target_link_libraries(simpleExample PRIVATE GraphLibrary::GraphLibrary)

target_include_directories(simpleExample PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_features(simpleExample PRIVATE cxx_std_20)
