cmake_minimum_required(VERSION 3.14)
project(Waves)

add_executable_with_shaders(Waves)
target_include_directories(Waves PRIVATE . )
set_target_properties(Waves PROPERTIES
    CXX_STANDARD 20
)
add_dependencies(Waves RealEngine)
target_link_libraries(Waves
    PUBLIC
        RealEngine
)
add_custom_command(TARGET Waves POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/data/ .
)

add_subdirectory(Waves)
