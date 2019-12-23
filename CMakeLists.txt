cmake_minimum_required(VERSION 3.5)
set (CMAKE_CXX_STANDARD 17)
project(discovery)

find_package(Vulkan REQUIRED)

add_library(glad vendor/include/glad/glad.c)

set(BUILD_SHARED_LIBS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(vendor/glfw)

file(GLOB DISCOVERY_src
    "src/*.cpp"
    "src/*/*.cpp"
)

add_library(discovery_lib ${DISCOVERY_src})

add_executable(discovery entry_point.cpp)

set(VENDOR_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/src"
    ${Vulkan_INCLUDE_DIRS}
)

target_include_directories(discovery_lib PRIVATE 
    ${VENDOR_INCLUDES}
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_include_directories(discovery PRIVATE 
    ${VENDOR_INCLUDES}
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(discovery_lib 
    glad
    glfw
    ${Vulkan_LIBRARIES}
)

target_link_libraries(discovery 
    discovery_lib
)

# copy contents of res/ into the directory of the output executable
add_custom_command(TARGET discovery PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E remove_directory
                    $<TARGET_FILE_DIR:discovery>/res)
                    
add_custom_command(TARGET discovery PRE_BUILD 
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/res $<TARGET_FILE_DIR:discovery>/res)