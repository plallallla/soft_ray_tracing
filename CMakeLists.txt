cmake_minimum_required(VERSION 3.10.0)
project(soft_ray_tracing VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(glm CONFIG REQUIRED)

add_executable(soft_ray_tracing src/main.cpp)

target_link_libraries(soft_ray_tracing PRIVATE glm::glm)
