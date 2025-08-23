# Contributing Guidelines

## Overview

This document outlines the development practices, coding standards, and contribution workflow for the C++ Satellite Tracking Library. All contributors must follow these guidelines to ensure code quality and maintainability.

## Commit Conventions

### Conventional Commits Format
We use [Conventional Commits](https://www.conventionalcommits.org/) specification:

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

### Commit Types
- **feat**: New feature implementation
- **fix**: Bug fixes
- **docs**: Documentation changes
- **style**: Code formatting (no logic changes)
- **refactor**: Code restructuring without behavior changes
- **test**: Adding or modifying tests
- **chore**: Build system, dependencies, or tooling changes
- **perf**: Performance improvements
- **ci**: CI/CD pipeline changes

### Examples
```bash
feat(sgp4): implement basic SGP4 propagation algorithm
fix(tle): handle malformed TLE parsing edge case
docs(api): add coordinate transformation examples
test(math): add unit tests for vector operations
chore(cmake): update minimum CMake version to 3.16
```

### Scope Guidelines
- **sgp4**: SGP4/SDP4 propagation algorithms
- **tle**: Two-Line Element processing
- **math**: Mathematical utilities and operations
- **coords**: Coordinate system transformations
- **time**: Time system handling
- **api**: Public API changes
- **build**: Build system and configuration
- **test**: Testing infrastructure

## C++ Coding Standards

### Language Features

#### Required C++17 Features
```cpp
// Use structured bindings
auto [x, y, z] = position.components();

// Use std::optional for nullable values
std::optional<Satellite> findSatellite(int noradId);

// Use constexpr if for compile-time branching
template<typename T>
constexpr T abs(T value) {
    if constexpr (std::is_unsigned_v<T>) {
        return value;
    } else {
        return value < 0 ? -value : value;
    }
}
```

#### Allowed C++20 Features (When Available)
```cpp
// Use concepts for template constraints
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

// Use ranges for algorithm composition
auto validSatellites = satellites 
    | std::views::filter([](const auto& sat) { return sat.isValid(); })
    | std::views::transform([](const auto& sat) { return sat.getPosition(); });
```

### Memory Management

#### Smart Pointers (Required)
```cpp
// Use unique_ptr for exclusive ownership
std::unique_ptr<SgpModel> createModel(const TleData& tle);

// Use shared_ptr for shared ownership
std::shared_ptr<EarthModel> earthModel = std::make_shared<Wgs84Model>();

// Use weak_ptr to break cycles
class GroundStation {
    std::weak_ptr<Satellite> trackedSatellite;
};
```

#### RAII Pattern (Required)
```cpp
class FileReader {
public:
    explicit FileReader(const std::string& filename) 
        : file_(filename) {
        if (!file_.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
    }
    
    ~FileReader() = default; // File closes automatically
    
private:
    std::ifstream file_;
};
```

### Error Handling

#### Exception Guidelines
```cpp
// Define custom exception types
class TleParseError : public std::runtime_error {
public:
    explicit TleParseError(const std::string& message) 
        : std::runtime_error("TLE Parse Error: " + message) {}
};

// Use exceptions for exceptional conditions
Satellite parseTle(const std::string& line1, const std::string& line2) {
    if (line1.length() != 69) {
        throw TleParseError("Line 1 must be exactly 69 characters");
    }
    // ... parsing logic
}

// Use std::optional for expected failures
std::optional<double> parseDouble(const std::string& str) noexcept {
    try {
        return std::stod(str);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}
```

### Naming Conventions

#### Classes and Types
```cpp
class SatelliteTracker;          // PascalCase
struct Vector3d;                 // PascalCase
enum class CoordinateSystem;     // PascalCase
using TimeStamp = std::chrono::time_point<std::chrono::utc_clock>;
```

#### Functions and Variables
```cpp
void calculatePosition();        // camelCase
double meanMotion;              // camelCase
const double EARTH_RADIUS_KM = 6378.137;  // SCREAMING_SNAKE_CASE for constants
```

#### Namespaces
```cpp
namespace satellite_tracking {   // snake_case
namespace sgp4 {                // snake_case
namespace detail {              // snake_case for internal implementation
```

### Code Organization

#### Header Structure
```cpp
#pragma once

// System includes
#include <memory>
#include <string>
#include <vector>

// Third-party includes
#include <eigen3/Eigen/Dense>

// Project includes
#include "satellite_tracking/types.h"
#include "satellite_tracking/math/vector3d.h"

namespace satellite_tracking {

class Satellite {
public:
    // Public interface
    
private:
    // Private implementation
};

} // namespace satellite_tracking
```

#### Implementation Structure
```cpp
#include "satellite_tracking/satellite.h"

// Additional includes as needed
#include <algorithm>
#include <cmath>

namespace satellite_tracking {

// Implementation details

} // namespace satellite_tracking
```

## Banned Practices

### Absolutely Forbidden
```cpp
// ❌ Raw pointers for ownership
int* data = new int[100];  // Use std::vector<int> or std::array

// ❌ Manual memory management
delete[] data;  // Use RAII instead

// ❌ C-style casts
double d = (double)integer;  // Use static_cast<double>(integer)

// ❌ Macros for constants
#define PI 3.14159  // Use constexpr double PI = 3.14159;

// ❌ Global variables
int globalCounter;  // Use dependency injection or singletons

// ❌ C-style arrays
int array[100];  // Use std::array<int, 100> or std::vector<int>
```

### Discouraged Patterns
```cpp
// ⚠️ Avoid deep template recursion
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N-1>::value;
};

// ⚠️ Avoid complex template metaprogramming
// Keep templates simple and readable

// ⚠️ Avoid platform-specific code
#ifdef _WIN32
    // Windows-specific code
#endif
// Use standard library or abstraction layers instead
```

## Review Process

### Pre-Commit Requirements
1. **Formatting**: Code must pass `clang-format` check
2. **Static Analysis**: Must pass `clang-tidy` and `cppcheck`
3. **Compilation**: Must compile without warnings on GCC, Clang, and MSVC
4. **Tests**: All existing tests must pass
5. **Coverage**: New code must have >90% test coverage

### Review Gates
1. **Automated Checks**: CI pipeline must pass
2. **Code Review**: At least one approving review required
3. **Architecture Review**: Required for API changes or new components
4. **Performance Review**: Required for performance-critical code

### Review Checklist
- [ ] Follows coding standards and conventions
- [ ] Includes comprehensive unit tests
- [ ] Documentation updated (API docs, README, etc.)
- [ ] No memory leaks or undefined behavior
- [ ] Thread-safety considered where applicable
- [ ] Error handling appropriate and consistent
- [ ] Performance impact assessed

## Testing Requirements

### Unit Testing
```cpp
#include <gtest/gtest.h>
#include "satellite_tracking/math/vector3d.h"

class Vector3dTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
    }
};

TEST_F(Vector3dTest, DefaultConstructor) {
    Vector3d v;
    EXPECT_DOUBLE_EQ(v.x(), 0.0);
    EXPECT_DOUBLE_EQ(v.y(), 0.0);
    EXPECT_DOUBLE_EQ(v.z(), 0.0);
}

TEST_F(Vector3dTest, Magnitude) {
    Vector3d v(3.0, 4.0, 0.0);
    EXPECT_DOUBLE_EQ(v.magnitude(), 5.0);
}
```

### Integration Testing
- Test complete workflows (TLE parsing → propagation → coordinate conversion)
- Validate against reference implementations
- Test error conditions and edge cases

### Performance Testing
```cpp
#include <benchmark/benchmark.h>

static void BM_SgpPropagation(benchmark::State& state) {
    // Setup
    for (auto _ : state) {
        // Benchmark code
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_SgpPropagation);
```

## Documentation Standards

### API Documentation (Doxygen)
```cpp
/**
 * @brief Calculates satellite position at given time
 * 
 * @param julianDate Time in Julian Date format
 * @param satellite Satellite object with orbital elements
 * @return Position vector in ECI coordinates (km)
 * 
 * @throws std::invalid_argument if julianDate is invalid
 * @throws PropagationError if propagation fails
 * 
 * @example
 * ```cpp
 * auto position = calculatePosition(2459945.5, satellite);
 * std::cout << "Position: " << position << std::endl;
 * ```
 */
Vector3d calculatePosition(double julianDate, const Satellite& satellite);
```

### README Updates
- Update examples when API changes
- Document new features and breaking changes
- Maintain performance benchmarks

## Build System

### CMake Standards
```cmake
# Minimum version
cmake_minimum_required(VERSION 3.16)

# Modern target-based approach
add_library(satellite_tracking)
target_sources(satellite_tracking PRIVATE src/satellite.cpp)
target_include_directories(satellite_tracking 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(satellite_tracking PUBLIC cxx_std_17)
```

### Dependency Management
- Use `find_package()` for system libraries
- Use `FetchContent` for header-only libraries
- Document all dependencies in README.md

## Continuous Integration

### Pipeline Requirements
- Build on Linux (GCC, Clang), Windows (MSVC), macOS (Clang)
- Run all tests with coverage reporting
- Static analysis with multiple tools
- Performance regression detection
- Documentation generation

### Quality Gates
- Zero compiler warnings
- All tests passing
- >90% code coverage
- Static analysis clean
- Performance within 5% of baseline

## Getting Started

1. **Fork** the repository
2. **Clone** your fork locally
3. **Create** a feature branch: `git checkout -b feat/my-feature`
4. **Implement** your changes following these guidelines
5. **Test** thoroughly with unit and integration tests
6. **Commit** using conventional commit format
7. **Push** to your fork and create a pull request

## Questions and Support

- Create an issue for bugs or feature requests
- Use discussions for questions and design proposals
- Tag maintainers for urgent issues

---

**Last Updated**: 2025-08-24  
**Version**: 1.0
