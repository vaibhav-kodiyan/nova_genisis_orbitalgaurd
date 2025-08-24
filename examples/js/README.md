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

The example demonstrates:
- Loading the WASM module
- Parsing TLE data (`og_parse_tle`)
- Propagating satellite positions (`og_propagate`)
- Calculating fuel consumption (`og_fuel_consumption`)
- Proper memory management

## Limitations

This is an **experimental** WASM build with the following limitations:
- Single-threaded execution
- No file I/O support
- Limited error handling
- Memory management requires manual `malloc`/`free`

For production applications, consider using the native C++ library or C API directly.
