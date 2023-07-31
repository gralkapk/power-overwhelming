# CMakeLists.txt
# Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.

cmake_minimum_required(VERSION 3.21.0)

project(posol)

# Collect source files.
file(GLOB_RECURSE HeaderFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h" "*.inl")
file(GLOB_RECURSE SourceFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cpp")

# Define the output.
add_library(${PROJECT_NAME} ${HeaderFiles} ${SourceFiles})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Configure the linker.
target_link_libraries(${PROJECT_NAME} power_overwhelming)

# lua
find_package(lua CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} lua)

# Sol
find_package(sol2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} sol2)
