cmake_minimum_required(VERSION 3.20...3.27)

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

# Optional: WebAssembly build
option(OG_BUILD_WASM "Build WebAssembly module" OFF)
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
    src/time_utils.cpp
    src/maneuver.cpp
)

# API source files
set(API_SOURCES
    ${SOURCES}
    src/api.cpp
)

# Create static library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Create API libraries (both static and shared)
add_library(oglib_static STATIC ${API_SOURCES})
add_library(oglib_shared SHARED ${API_SOURCES})

# Set library properties
set_target_properties(oglib_static PROPERTIES
    OUTPUT_NAME oglib
    PUBLIC_HEADER "include/api.h"
)

set_target_properties(oglib_shared PROPERTIES
    OUTPUT_NAME oglib
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "include/api.h"
)

# Set symbol visibility for shared library
if(NOT WIN32)
    set_target_properties(oglib_shared PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
endif()

# Windows DLL export
if(WIN32)
    set_target_properties(oglib_shared PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

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

# WebAssembly build support
if(OG_BUILD_WASM AND EMSCRIPTEN)
    message(STATUS "Building WebAssembly module with Emscripten")
    
    # Create WASM library with API
    add_executable(oglib_wasm ${API_SOURCES})
    set_target_properties(oglib_wasm PROPERTIES
        SUFFIX ".js"
        LINK_FLAGS "-s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME='OrbitalGuard' -s EXPORTED_FUNCTIONS=_og_parse_tle,_og_free_elements,_og_propagate,_og_screen,_og_plan_maneuver,_og_fuel_consumption,_og_last_error -s EXPORTED_RUNTIME_METHODS=malloc,free"
    )
    
    # Optional: Add environment and other flags
    set_target_properties(oglib_wasm PROPERTIES
        LINK_FLAGS "${CMAKE_TARGET_PROPERTY_LINK_FLAGS} -s ENVIRONMENT=web,node"
    )
    
    message(STATUS "WASM build configured. Use 'make oglib_wasm' to build.")
elseif(OG_BUILD_WASM AND NOT EMSCRIPTEN)
    message(WARNING "OG_BUILD_WASM=ON but Emscripten toolchain not detected. Skipping WASM build.")
endif()

# Installation
install(TARGETS ${PROJECT_NAME} oglib_static oglib_shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
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

add_executable(test_api tests/test_api.cpp)
target_link_libraries(test_api oglib_static)

add_executable(test_maneuver tests/test_maneuver.cpp)
target_link_libraries(test_maneuver ${PROJECT_NAME})

# Add tests to CTest
add_test(NAME ConstantsTest COMMAND test_constants)
add_test(NAME TypesTest COMMAND test_types)
add_test(NAME PropagationTest COMMAND test_propagation)
add_test(NAME ScreeningTest COMMAND test_screening)
add_test(NAME APITest COMMAND test_api)
add_test(NAME ManeuverTest COMMAND test_maneuver)
