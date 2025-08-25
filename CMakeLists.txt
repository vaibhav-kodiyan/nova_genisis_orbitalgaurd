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

# Optional: SGP4 integration
option(ENABLE_SGP4 "Enable SGP4 propagation support" ON)
if(ENABLE_SGP4)
    include(FetchContent)
    FetchContent_Declare(
        sgp4_ext
        GIT_REPOSITORY https://github.com/magnific0/SGP4.git
        # TODO: Pin to a specific commit for full reproducibility, e.g., GIT_TAG <commit-hash>
        GIT_TAG master
        CMAKE_ARGS
            -DSGP4_WITH_TESTS=OFF
    )
    # Ensure cache in dependency is set even if previously configured
    set(SGP4_WITH_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(sgp4_ext)
endif()

# Source files
set(SOURCES
    src/types.cpp
    src/propagation.cpp
    src/screening.cpp
    src/maneuver.cpp
)

# Create static library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Link SGP4 to library and expose includes when enabled
if(ENABLE_SGP4)
    # The fetched project defines a static target named SGP4
    target_link_libraries(${PROJECT_NAME} PUBLIC SGP4)
    # Expose the fetched source dir so includes like "SGP4/SGP4/SGP4.h" resolve
    if(DEFINED sgp4_ext_SOURCE_DIR)
        target_include_directories(${PROJECT_NAME} PUBLIC ${sgp4_ext_SOURCE_DIR})
    endif()
endif()

# Optional demo executable (JSON generation) that depends on external SGP4
if(ENABLE_SGP4)
    add_executable(${PROJECT_NAME}_test src/main.cpp)
    # Link library and also SGP4 when enabled (use keyword signature consistently)
    target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME})
    # Also link SGP4 directly to the test exe (in case it includes headers directly)
    target_link_libraries(${PROJECT_NAME}_test PRIVATE SGP4)
endif()

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

# Enable testing
enable_testing()

# Add test executable
add_executable(test_constants tests/test_constants.cpp)
target_link_libraries(test_constants ${PROJECT_NAME})

add_executable(test_types tests/test_types.cpp)
target_link_libraries(test_types ${PROJECT_NAME})

add_executable(test_propagation tests/test_propagation.cpp)
target_link_libraries(test_propagation ${PROJECT_NAME})

add_executable(test_screening tests/test_screening.cpp)
target_link_libraries(test_screening ${PROJECT_NAME})

add_executable(test_maneuver tests/test_maneuver.cpp)
target_link_libraries(test_maneuver ${PROJECT_NAME})

# Add tests to CTest
add_test(NAME ConstantsTest COMMAND test_constants)
add_test(NAME TypesTest COMMAND test_types)
add_test(NAME PropagationTest COMMAND test_propagation)
add_test(NAME ScreeningTest COMMAND test_screening)
add_test(NAME ManeuverTest COMMAND test_maneuver)
