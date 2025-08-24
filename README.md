# C++ Satellite Tracking Library

A high-performance C++ library for satellite orbit propagation and tracking, ported from TypeScript with modern C++ practices.

## Project Status

**Current Phase**: 3 - Core Constants & Types Porting ✅  
**Next Phase**: 4 - Time Systems

## Quick Start

### Building the Project

#### Prerequisites
- CMake 3.20 or higher
- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)

#### Native Build
```bash
# Configure the build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build build

# Run the test executable
./build/nova_genesis_orbitalguard_test
```

#### Debug Build
```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

#### Shared Library Build
```bash
# Build both static and shared libraries with C API
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Libraries will be built as:
# - liboglib.a (static)
# - liboglib.so (shared, Linux/macOS) or oglib.dll (Windows)
```

#### WebAssembly Build (Experimental)
```bash
# Requires Emscripten SDK
emcmake cmake -B build-wasm -S . -DOG_BUILD_WASM=ON
cmake --build build-wasm --target oglib_wasm

# Generates oglib_wasm.js and oglib_wasm.wasm
```

### Library Features

- **SGP4/SDP4 Propagation**: High-accuracy satellite position prediction ✅
- **TLE Processing**: Two-Line Element parsing and validation ✅
- **C API**: Stable C ABI for cross-language interoperability ✅
- **WebAssembly**: Experimental WASM build support ✅
- **Conjunction Screening**: Satellite collision detection ✅
- **Maneuver Planning**: Basic collision avoidance (simplified) ✅
- **Coordinate Transformations**: ECI, ECEF, and topocentric conversions (planned)
- **Ground Station Calculations**: Azimuth, elevation, range, and Doppler (planned)
- **Modern C++**: C++20 with performance optimizations

## Repository Structure

```
├── docs/                   # Project documentation
│   ├── CHARTER.md         # Project scope and constraints
│   ├── CONTRIBUTING.md    # Development guidelines
│   └── PHASES.md          # Development roadmap
├── include/               # Public header files
│   └── api.h              # C API interface
├── src/                   # Implementation files
├── tests/                 # Unit and integration tests
├── data/                  # Test data and samples
│   └── sample.tle         # Sample TLE data for testing
└── archive/               # Historical documentation
```

## Development Phases

1. **✅ Baseline & Charter** - Project setup and documentation
2. **✅ Core Constants & Types** - Physical constants and data structures
3. **✅ Time Systems** - Julian dates and time conversions
4. **✅ TLE Processing** - Two-Line Element parsing
5. **✅ SGP4 Implementation** - Core propagation algorithms
6. **✅ C API & WebAssembly** - Cross-language interoperability
7. **📋 Coordinate Transforms** - Reference frame conversions
8. **📋 Ground Station** - Observer calculations
9. **📋 Testing & Validation** - Comprehensive test suite
10. **📋 Performance Optimization** - Profiling and tuning
11. **📋 Documentation** - API docs and examples

## C API

The library provides a stable C ABI through `include/api.h` with the following exported symbols:

- `og_parse_tle()` - Parse TLE data into orbital elements
- `og_free_elements()` - Free orbital elements handle
- `og_propagate()` - Propagate satellite position (minutes input)
- `og_screen()` - Screen for satellite conjunctions
- `og_plan_maneuver()` - Plan collision avoidance maneuvers
- `og_fuel_consumption()` - Calculate fuel requirements
- `og_last_error()` - Get last error message

### Units and Time Base
- **Positions**: kilometers (km)
- **Velocities**: kilometers per second (km/s)
- **Time**: Julian dates for epochs, minutes for propagation offsets
- **Delta-V**: meters per second (m/s) in ECI coordinates

### Ownership Rules
- Handles returned by `og_parse_tle()` must be freed with `og_free_elements()`
- Output arrays are caller-owned
- Error strings are library-owned (do not free)

### Thread Safety
- API functions are stateless but use thread-local error storage
- Internal propagation algorithms are thread-safe

## Requirements

- **C++ Standard**: C++17 minimum, C++20 preferred
- **Compilers**: GCC 9+, Clang 10+, MSVC 2019+
- **Build System**: CMake 3.16+
- **Testing**: Google Test (will be added in Phase 8)

## Performance Goals

- **Speed**: >1M satellite propagations/second
- **Memory**: <1KB per tracked satellite
- **Accuracy**: <1m position error for 24-hour predictions
- **Improvement**: >10x faster than TypeScript version

## Contributing

Please read [CONTRIBUTING.md](docs/CONTRIBUTING.md) for development guidelines, coding standards, and commit conventions.

## Documentation

- **[Charter](docs/CHARTER.md)** - Project scope and technical constraints
- **[Contributing](docs/CONTRIBUTING.md)** - Development guidelines
- **[Phases](docs/PHASES.md)** - Detailed development roadmap
- **[Archive](archive/)** - Historical TypeScript documentation

## License

[License to be determined]

## Contact

[Contact information to be added]

---

**Status**: Phase 3 Complete - Ready for Time Systems  
**Last Updated**: 2025-08-24
