/**
 * Minimal WebAssembly example for OrbitalGuard library
 * 
 * This is an experimental example demonstrating how to load and use
 * the OrbitalGuard WASM module in Node.js or browser environments.
 * 
 * Prerequisites:
 * 1. Build the WASM module: cmake -B build-wasm -DOG_BUILD_WASM=ON && make oglib_wasm
 * 2. Copy oglib_wasm.js and oglib_wasm.wasm to this directory
 * 3. Run with Node.js: node example.js
 */

// Sample ISS TLE data for testing
const ISS_TLE = {
    name: "ISS (ZARYA)",
    line1: "1 25544U 98067A   23001.00000000  .00002182  00000-0  40768-4 0  9992",
    line2: "2 25544  51.6461 339.2971 0002972  68.7102 291.5211 15.48919103123456"
};

async function loadOrbitalGuard() {
    try {
        // Load the WASM module (adjust path as needed)
        const OrbitalGuard = require('./oglib_wasm.js');
        
        // Initialize the module
        const og = await OrbitalGuard();
        
        console.log('✅ OrbitalGuard WASM module loaded successfully');
        return og;
    } catch (error) {
        console.error('❌ Failed to load OrbitalGuard WASM module:', error.message);
        console.log('Make sure to build the WASM module first:');
        console.log('  emcmake cmake -B build-wasm -DOG_BUILD_WASM=ON');
        console.log('  cmake --build build-wasm --target oglib_wasm');
        console.log('  cp build-wasm/oglib_wasm.* examples/js/');
        return null;
    }
}

function demonstrateAPI(og) {
    console.log('\n=== OrbitalGuard API Demonstration ===\n');
    
    try {
        // 1. Parse TLE data
        console.log('1. Parsing TLE data...');
        const elements = og.ccall('og_parse_tle', 'number', ['string', 'string', 'string'], 
            [ISS_TLE.name, ISS_TLE.line1, ISS_TLE.line2]);
        
        if (!elements) {
            const error = og.ccall('og_last_error', 'string', [], []);
            console.error('❌ TLE parsing failed:', error);
            return;
        }
        console.log('✅ TLE parsed successfully, handle:', elements);
        
        // 2. Propagate satellite position
        console.log('\n2. Propagating satellite position (90 minutes)...');
        const posPtr = og._malloc(3 * 8); // 3 doubles for position
        const velPtr = og._malloc(3 * 8); // 3 doubles for velocity
        
        const result = og.ccall('og_propagate', 'number', ['number', 'number', 'number', 'number'],
            [elements, 90.0, posPtr, velPtr]);
        
        if (result === 0) {
            // Read position and velocity from memory
            const position = [
                og.getValue(posPtr, 'double'),
                og.getValue(posPtr + 8, 'double'),
                og.getValue(posPtr + 16, 'double')
            ];
            const velocity = [
                og.getValue(velPtr, 'double'),
                og.getValue(velPtr + 8, 'double'),
                og.getValue(velPtr + 16, 'double')
            ];
            
            console.log('✅ Propagation successful:');
            console.log(`   Position: [${position.map(x => x.toFixed(2)).join(', ')}] km`);
            console.log(`   Velocity: [${velocity.map(x => x.toFixed(4)).join(', ')}] km/s`);
        } else {
            const error = og.ccall('og_last_error', 'string', [], []);
            console.error('❌ Propagation failed:', error);
        }
        
        // Clean up memory
        og._free(posPtr);
        og._free(velPtr);
        
        // 3. Test fuel consumption calculation
        console.log('\n3. Testing fuel consumption calculation...');
        const fuelRequired = og.ccall('og_fuel_consumption', 'number', 
            ['number', 'number', 'number', 'number', 'number'],
            [0.1, 300.0, 1000.0, 200.0, 0.9]); // 0.1 km/s, 300s Isp, 1000kg dry, 200kg fuel, 90% efficiency
        
        if (fuelRequired >= 0) {
            console.log(`✅ Fuel required: ${fuelRequired.toFixed(2)} kg`);
        } else {
            const error = og.ccall('og_last_error', 'string', [], []);
            console.error('❌ Fuel calculation failed:', error);
        }
        
        // 4. Clean up
        console.log('\n4. Cleaning up...');
        og.ccall('og_free_elements', 'void', ['number'], [elements]);
        console.log('✅ Resources freed');
        
    } catch (error) {
        console.error('❌ API demonstration failed:', error.message);
    }
}

async function main() {
    console.log('OrbitalGuard WebAssembly Example');
    console.log('================================\n');
    
    const og = await loadOrbitalGuard();
    if (og) {
        demonstrateAPI(og);
    }
    
    console.log('\n=== Example Complete ===');
    console.log('Note: This is an experimental WASM build.');
    console.log('For production use, consider the native C++ library.');
}

// Run the example
if (require.main === module) {
    main().catch(console.error);
}

module.exports = { loadOrbitalGuard, demonstrateAPI };
