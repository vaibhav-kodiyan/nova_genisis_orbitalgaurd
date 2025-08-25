cmake_minimum_required(VERSION 3.20...3.27)

# Set modern CMake policies to avoid warnings
cmake_policy(SET CMP0135 NEW)  # Use DOWNLOAD_EXTRACT_TIMESTAMP for FetchContent

# Project configuration
project(nova_genesis_orbitalguard 
    VERSION 1.0.0
    DESCRIPTION "Procedural C++ library for orbital mechanics"
    LANGUAGES CXX
)

# Set C++17 standard for simplified core
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add simplified core compilation flag
add_compile_definitions(SIMPLIFIED_CORE=1)

# Compiler-specific flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# Build types
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Include directories
include_directories(include)

# No external dependencies (SGP4/WASM removed for simplified build)

# Source files
set(SOURCES
    src/types.cpp
    src/propagation.cpp
    src/screening.cpp
    src/maneuver.cpp
)

# Do not build C ABI or WASM bindings in simplified pipeline

# Create static library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Removed API libraries

# No SGP4 linking required

# JSON generator executable (always build)
add_executable(${PROJECT_NAME}_test src/main.cpp)
target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME})

# Post-build: copy JSON outputs to frontend/public (if they exist)
add_custom_command(TARGET ${PROJECT_NAME}_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/frontend/public
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/tests/coordinates.json
            ${CMAKE_SOURCE_DIR}/frontend/public/coordinates.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/tests/conjunctions.json
            ${CMAKE_SOURCE_DIR}/frontend/public/conjunctions.json
    COMMENT "Copying JSON outputs to frontend/public"
)

# WASM build removed in simplified pipeline

# Installation
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
