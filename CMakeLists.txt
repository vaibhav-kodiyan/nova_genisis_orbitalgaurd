cmake_minimum_required(VERSION 3.20)

# Project configuration
project(nova_genesis_orbitalguard 
    VERSION 1.0.0
    DESCRIPTION "Procedural C++ library for orbital mechanics"
    LANGUAGES CXX
)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# Build types
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/constants.cpp
    src/types.cpp
)

# Create static library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Optional executable for testing
add_executable(${PROJECT_NAME}_test src/main.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})

# Emscripten detection and WebAssembly support
if(EMSCRIPTEN)
    message(STATUS "Building with Emscripten for WebAssembly")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        COMPILE_FLAGS "-s USE_PTHREADS=0"
        LINK_FLAGS "-s WASM=1 -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
    )
    
    # Create WASM library
    add_executable(${PROJECT_NAME}_wasm ${SOURCES})
    set_target_properties(${PROJECT_NAME}_wasm PROPERTIES
        SUFFIX ".js"
        LINK_FLAGS "-s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME='NovaGenesisOrbitalGuard'"
    )
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
