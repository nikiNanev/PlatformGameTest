cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(platform)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(glm REQUIRED)

set(CMAKE_BUILD_TYPE Debug)


add_library(stb_image INTERFACE)

include_directories(./include)

add_executable(${PROJECT_NAME} src/main.cpp)


target_link_libraries(${PROJECT_NAME} PRIVATE fmt stb_image SDL3::SDL3 SDL3_mixer::SDL3_mixer SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf glm::glm WavPack::WavPack)
