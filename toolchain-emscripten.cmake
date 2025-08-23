# Emscripten toolchain file for WebAssembly builds
# Usage: emcmake cmake -B build-wasm -S . -DCMAKE_TOOLCHAIN_FILE=toolchain-emscripten.cmake

set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_SYSTEM_VERSION 1)

# Emscripten paths (will be set by emcmake)
set(CMAKE_C_COMPILER "emcc")
set(CMAKE_CXX_COMPILER "em++")
set(CMAKE_AR "emar")
set(CMAKE_RANLIB "emranlib")

# Emscripten-specific flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_PTHREADS=0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")

# WebAssembly specific settings
set(CMAKE_EXECUTABLE_SUFFIX ".js")

# Find root path for Emscripten
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Optimization flags for different build types
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
