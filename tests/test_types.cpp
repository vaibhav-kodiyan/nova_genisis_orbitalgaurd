#include <cassert>
#include <cstring>
#include <iostream>
#include "../include/types.h"

void test_tle_struct() {
    // Test TLE struct size and layout
    TLE tle;
    
    // Verify struct has expected size
    static_assert(sizeof(tle.name) == 24, "TLE name field should be 24 bytes");
    static_assert(sizeof(tle.line1) == 70, "TLE line1 field should be 70 bytes");
    static_assert(sizeof(tle.line2) == 70, "TLE line2 field should be 70 bytes");
    
    // Test field initialization
    memset(&tle, 0, sizeof(tle));
    strcpy(tle.name, "ISS (ZARYA)");
    strcpy(tle.line1, "1 25544U 98067A   08264.51782528 -.00002182  00000-0 -11606-4 0  2927");
    strcpy(tle.line2, "2 25544  51.6416 247.4627 0006703 130.5360 325.0288 15.72125391563537");
    
    assert(strlen(tle.name) > 0);
    assert(strlen(tle.line1) == 69);
    assert(strlen(tle.line2) == 69);
}

void test_orbital_elements_struct() {
    // Test OrbitalElements struct
    OrbitalElements elements;
    memset(&elements, 0, sizeof(elements));
    
    // Verify all fields are double precision
    static_assert(sizeof(elements.epoch) == sizeof(double), "epoch should be double");
    static_assert(sizeof(elements.mean_motion) == sizeof(double), "mean_motion should be double");
    static_assert(sizeof(elements.eccentricity) == sizeof(double), "eccentricity should be double");
    static_assert(sizeof(elements.inclination) == sizeof(double), "inclination should be double");
    static_assert(sizeof(elements.raan) == sizeof(double), "raan should be double");
    static_assert(sizeof(elements.arg_perigee) == sizeof(double), "arg_perigee should be double");
    static_assert(sizeof(elements.mean_anomaly) == sizeof(double), "mean_anomaly should be double");
    static_assert(sizeof(elements.bstar) == sizeof(double), "bstar should be double");
    static_assert(sizeof(elements.ndot) == sizeof(double), "ndot should be double");
    static_assert(sizeof(elements.nddot) == sizeof(double), "nddot should be double");
    
    // Test field assignment
    elements.epoch = 2458264.51782528;
    elements.mean_motion = 15.72125391;
    elements.eccentricity = 0.0006703;
    elements.inclination = 0.9006;
    
    assert(elements.epoch > 0);
    assert(elements.mean_motion > 0);
    assert(elements.eccentricity >= 0 && elements.eccentricity < 1);
}

void test_state_vector_struct() {
    // Test StateVectorECI struct
    StateVectorECI state;
    memset(&state, 0, sizeof(state));
    
    // Verify array sizes
    static_assert(sizeof(state.r) == 3 * sizeof(double), "position vector should be 3 doubles");
    static_assert(sizeof(state.v) == 3 * sizeof(double), "velocity vector should be 3 doubles");
    
    // Test field assignment
    state.t = 2458264.51782528;
    state.r[0] = -6045.0;
    state.r[1] = -3490.0;
    state.r[2] = 2500.0;
    state.v[0] = -3.457;
    state.v[1] = 6.618;
    state.v[2] = 2.533;
    
    assert(state.t > 0);
    assert(state.r[0] != 0 || state.r[1] != 0 || state.r[2] != 0);
    assert(state.v[0] != 0 || state.v[1] != 0 || state.v[2] != 0);
}

void test_encounter_struct() {
    // Test Encounter struct
    Encounter encounter;
    memset(&encounter, 0, sizeof(encounter));
    
    // Verify field sizes
    static_assert(sizeof(encounter.id) == 64, "encounter id should be 64 bytes");
    static_assert(sizeof(encounter.severity) == sizeof(int), "severity should be int");
    
    // Test field assignment
    strcpy(encounter.id, "SAT1_SAT2_20240824_123456");
    encounter.tca = 2458264.51782528;
    encounter.distance = 0.5;
    encounter.relative_velocity = 12.3;
    encounter.severity = SEVERITY_CLOSE;
    encounter.probability = 1e-6;
    
    assert(strlen(encounter.id) > 0);
    assert(encounter.distance >= 0);
    assert(encounter.relative_velocity >= 0);
    assert(encounter.severity >= SEVERITY_COLLISION && encounter.severity <= SEVERITY_NOMINAL);
    assert(encounter.probability >= 0 && encounter.probability <= 1);
}

void test_severity_constants() {
    // Test severity constant values
    assert(SEVERITY_COLLISION == 0);
    assert(SEVERITY_CRITICAL == 1);
    assert(SEVERITY_CLOSE == 2);
    assert(SEVERITY_NOMINAL == 3);
    
    // Test severity_to_string function
    assert(strcmp(severity_to_string(SEVERITY_COLLISION), "COLLISION") == 0);
    assert(strcmp(severity_to_string(SEVERITY_CRITICAL), "CRITICAL") == 0);
    assert(strcmp(severity_to_string(SEVERITY_CLOSE), "CLOSE") == 0);
    assert(strcmp(severity_to_string(SEVERITY_NOMINAL), "NOMINAL") == 0);
    assert(strcmp(severity_to_string(99), "UNKNOWN") == 0);
}

int main() {
    test_tle_struct();
    test_orbital_elements_struct();
    test_state_vector_struct();
    test_encounter_struct();
    test_severity_constants();
    
    std::cout << "All type tests passed!" << std::endl;
    return 0;
}
