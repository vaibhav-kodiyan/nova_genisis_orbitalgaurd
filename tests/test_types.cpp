#include <cassert>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include "../include/types.h"

// Deterministic test setup
void setup_deterministic_environment() {
    // Set deterministic seed for any random operations
    std::srand(42);
    
    // Ensure consistent locale
    setlocale(LC_ALL, "C");
}

int main() {
    setup_deterministic_environment();
    // Test Severity enum
    assert(NONE == 0);
    assert(LOW == 1);
    assert(MEDIUM == 2);
    assert(HIGH == 3);
    assert(CRASH == 4);
    
    // Test TLE struct
    TLE tle;
    tle.name = "TEST SAT";
    strcpy(tle.line1, "1 25544U 98067A   08264.51782528 -.00002182  00000-0 -11606-4 0  2927");
    strcpy(tle.line2, "2 25544  51.6416 247.4627 0006703 130.5360 325.0288 15.72125391563537");
    
    assert(tle.name == "TEST SAT");
    assert(strlen(tle.line1) > 0);
    assert(strlen(tle.line2) > 0);
    
    // Test OrbitalElements struct
    OrbitalElements elements;
    elements.semi_major_axis = 6800.0;
    elements.eccentricity = 0.01;
    elements.tilt = 0.9;
    elements.node = 1.5;
    elements.perigee_angle = 2.0;
    elements.position = 0.5;
    elements.time = 2458849.5;
    
    assert(elements.semi_major_axis == 6800.0);
    assert(elements.eccentricity == 0.01);
    
    // Test StateVectorECI struct
    StateVectorECI state;
    state.t = 2458849.5;
    state.r[0] = 6800.0; state.r[1] = 0.0; state.r[2] = 0.0;
    state.v[0] = 0.0; state.v[1] = 7.5; state.v[2] = 0.0;
    
    assert(state.r[0] == 6800.0);
    assert(state.v[1] == 7.5);
    
    // Test satellite_data struct
    satellite_data sat;
    sat.name = "TEST SATELLITE";
    sat.orbital_data = elements;
    sat.state_data = state;
    sat.tle = tle;
    
    assert(sat.name == "TEST SATELLITE");
    
    std::cout << "Types test passed!" << std::endl;
    return 0;
}
