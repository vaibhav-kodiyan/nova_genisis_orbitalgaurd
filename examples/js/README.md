# JavaScript WebAssembly Example

This directory contains an experimental example of using the OrbitalGuard library in JavaScript via WebAssembly.

## Prerequisites

1. **Emscripten SDK** installed and activated
2. **Build the WASM module**:
   ```bash
   emcmake cmake -B build-wasm -DOG_BUILD_WASM=ON
   cmake --build build-wasm --target oglib_wasm
   ```
3. **Copy WASM files** to this directory:
   ```bash
   cp build-wasm/oglib_wasm.js build-wasm/oglib_wasm.wasm examples/js/
   ```

## Running the Example

### Node.js
```bash
cd examples/js
node example.js
```

### Browser
1. Serve the files via HTTP (required for WASM loading):
   ```bash
   python -m http.server 8000
   ```
2. Open `http://localhost:8000` and load the example in browser console

## API Usage

The example demonstrates basic usage of the OrbitalGuard C API through WebAssembly:

1. **TLE Parsing**: Convert Two-Line Element sets to internal orbital elements
2. **Orbit Propagation**: Calculate satellite position and velocity at future times
3. **Maneuver Planning**: Plan collision avoidance maneuvers using Gregorian calendar time
4. **Fuel Consumption**: Estimate propellant requirements for maneuvers
5. **Memory Management**: Proper cleanup of allocated resources

Key API functions used:
- `og_parse_tle()` - Parse TLE data
- `og_propagate()` - Propagate orbit to specified time
- `og_plan_maneuver()` - Plan maneuvers with Gregorian time input
- `og_fuel_consumption()` - Calculate fuel requirements
- `og_free_elements()` - Clean up memory

**Time Format**: The API now uses Gregorian calendar time (`gregorian_time_t` structure) instead of Julian dates for better usability. Time structures contain year, month, day, hour, minute, and second fields.

## Limitations

This is an **experimental** WASM build with the following limitations:
- Single-threaded execution
- No file I/O support
- Limited error handling
- Memory management requires manual `malloc`/`free`

For production applications, consider using the native C++ library or C API directly.
