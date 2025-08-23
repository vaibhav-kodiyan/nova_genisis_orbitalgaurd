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

#### WebAssembly Build (Optional)
```bash
# Requires Emscripten SDK
emcmake cmake -B build-wasm -S . -DCMAKE_TOOLCHAIN_FILE=toolchain-emscripten.cmake
cmake --build build-wasm
```

### Library Features (Planned)

- **SGP4/SDP4 Propagation**: High-accuracy satellite position prediction
- **TLE Processing**: Two-Line Element parsing and validation  
- **Coordinate Transformations**: ECI, ECEF, and topocentric conversions
- **Ground Station Calculations**: Azimuth, elevation, range, and Doppler
- **Modern C++**: C++20 with performance optimizations

## Repository Structure

```
├── docs/                   # Project documentation
│   ├── CHARTER.md         # Project scope and constraints
│   ├── CONTRIBUTING.md    # Development guidelines
│   └── PHASES.md          # Development roadmap
├── include/               # Public header files
├── src/                   # Implementation files
├── tests/                 # Unit and integration tests
├── data/                  # Test data and samples
│   └── sample.tle         # Sample TLE data for testing
└── archive/               # Historical documentation
```

## Development Phases

1. **✅ Baseline & Charter** - Project setup and documentation
2. **✅ Core Constants & Types** - Physical constants and data structures
3. **📋 Time Systems** - Julian dates and time conversions
4. **📋 TLE Processing** - Two-Line Element parsing
5. **📋 SGP4 Implementation** - Core propagation algorithms
6. **📋 Coordinate Transforms** - Reference frame conversions
7. **📋 Ground Station** - Observer calculations
8. **📋 Testing & Validation** - Comprehensive test suite
9. **📋 Performance Optimization** - Profiling and tuning
10. **📋 Documentation** - API docs and examples

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
