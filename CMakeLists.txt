cmake_minimum_required(VERSION 3.5)
set (CMAKE_CXX_STANDARD 11)
project(DISCOVERY_GAME)

add_library(GLAD vendor/include/glad/glad.c)

set(BUILD_SHARED_LIBS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(vendor/glfw)

set(EASTL_BUILD_TESTS ON) # because EASTL build is currently broken when not building tests
add_subdirectory(vendor/EASTL)

file(GLOB DISCOVERY_src
    "src/*.cpp"
)

add_executable(DISCOVERY_GAME ${DISCOVERY_src})

set(VENDOR_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/EASTL/include"
)

target_include_directories(DISCOVERY_GAME PRIVATE 
    ${VENDOR_INCLUDES}
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(DISCOVERY_GAME
    EASTL
    GLAD
    glfw
)