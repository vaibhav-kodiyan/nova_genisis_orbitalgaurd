#include <iostream>
#include <cmath>
#include <cassert>
#include "../include/propagation.h"
#include "../include/constants.h"

// Test tolerance for floating point comparisons
#define TEST_TOLERANCE 1e-6

// Helper function to check if two doubles are approximately equal
bool approx_equal(double a, double b, double tolerance = TEST_TOLERANCE) {
    return fabs(a - b) < tolerance;
}

// Helper function to check if a vector has finite, non-zero values
bool is_valid_vector(const double vec[3]) {
    for (int i = 0; i < 3; i++) {
        if (!std::isfinite(vec[i]) || std::isnan(vec[i])) {
            return false;
        }
    }
    // Check if at least one component is non-zero
    return (fabs(vec[0]) > 1e-10 || fabs(vec[1]) > 1e-10 || fabs(vec[2]) > 1e-10);
}

// Test case 1: Basic propagation with ISS-like orbital elements
void test_basic_propagation() {
    std::cout << "Running test_basic_propagation..." << std::endl;
    
    // ISS-like orbital elements (approximate)
    OrbitalElements elements = {0};
    elements.epoch = 2460000.0;  // Julian date
    elements.mean_motion = 15.5;  // revolutions per day
    elements.eccentricity = 0.0001;  // Nearly circular
    elements.inclination = 51.6 * DEG_TO_RAD;  // ISS inclination
    elements.raan = 45.0 * DEG_TO_RAD;
    elements.arg_perigee = 30.0 * DEG_TO_RAD;
    elements.mean_anomaly = 0.0;
    elements.bstar = 0.0001;
    elements.ndot = 0.0;
    elements.nddot = 0.0;
    
    StateVectorECI state;
    
    // Test propagation at epoch (t=0)
    int result = propagate(&elements, 0.0, &state);
    assert(result == PROPAGATION_SUCCESS);
    assert(is_valid_vector(state.r));
    assert(is_valid_vector(state.v));
    
    // Check that position magnitude is reasonable for LEO
    double r_mag = sqrt(state.r[0]*state.r[0] + state.r[1]*state.r[1] + state.r[2]*state.r[2]);
    assert(r_mag > 6500.0 && r_mag < 7000.0);  // Typical LEO altitude range
    
    // Check that velocity magnitude is reasonable
    double v_mag = sqrt(state.v[0]*state.v[0] + state.v[1]*state.v[1] + state.v[2]*state.v[2]);
    assert(v_mag > 7.0 && v_mag < 8.0);  // Typical orbital velocity
    
    std::cout << "  Position: [" << state.r[0] << ", " << state.r[1] << ", " << state.r[2] << "] km" << std::endl;
    std::cout << "  Velocity: [" << state.v[0] << ", " << state.v[1] << ", " << state.v[2] << "] km/s" << std::endl;
    std::cout << "  PASSED" << std::endl;
}

// Test case 2: Propagation at different time offsets
void test_time_propagation() {
    std::cout << "Running test_time_propagation..." << std::endl;
    
    OrbitalElements elements = {0};
    elements.epoch = 2460000.0;
    elements.mean_motion = 15.0;
    elements.eccentricity = 0.01;
    elements.inclination = 45.0 * DEG_TO_RAD;
    elements.raan = 0.0;
    elements.arg_perigee = 0.0;
    elements.mean_anomaly = 0.0;
    // Note: bstar, ndot, nddot are not part of the current OrbitalElements struct
    
    StateVectorECI state1, state2, state3;
    
    // Test at different time offsets
    assert(propagate(&elements, 0.0, &state1) == PROPAGATION_SUCCESS);
    assert(propagate(&elements, 30.0, &state2) == PROPAGATION_SUCCESS);  // 30 minutes later
    assert(propagate(&elements, 90.0, &state3) == PROPAGATION_SUCCESS);  // 90 minutes later
    
    // All states should be valid
    assert(is_valid_vector(state1.r) && is_valid_vector(state1.v));
    assert(is_valid_vector(state2.r) && is_valid_vector(state2.v));
    assert(is_valid_vector(state3.r) && is_valid_vector(state3.v));
    
    // Positions should be different at different times
    double dist12 = sqrt(pow(state2.r[0] - state1.r[0], 2) + 
                        pow(state2.r[1] - state1.r[1], 2) + 
                        pow(state2.r[2] - state1.r[2], 2));
    assert(dist12 > 100.0);  // Should move significantly in 30 minutes
    
    std::cout << "  Position change over 30 min: " << dist12 << " km" << std::endl;
    std::cout << "  PASSED" << std::endl;
}

// Test case 3: Error handling for invalid inputs
void test_error_handling() {
    std::cout << "Running test_error_handling..." << std::endl;
    
    OrbitalElements elements = {0};
    StateVectorECI state;
    
    // Test null pointer inputs
    assert(propagate(nullptr, 0.0, &state) == PROPAGATION_ERROR_INVALID_INPUT);
    assert(propagate(&elements, 0.0, nullptr) == PROPAGATION_ERROR_INVALID_INPUT);
    
    // Test invalid eccentricity (>= 1.0)
    elements.eccentricity = 1.5;
    elements.mean_motion = 15.0;
    assert(propagate(&elements, 0.0, &state) == PROPAGATION_ERROR_INVALID_INPUT);
    
    // Test negative eccentricity
    elements.eccentricity = -0.1;
    assert(propagate(&elements, 0.0, &state) == PROPAGATION_ERROR_INVALID_INPUT);
    
    std::cout << "  PASSED" << std::endl;
}

// Test case 4: High eccentricity orbit (but still < 1.0)
void test_high_eccentricity() {
    std::cout << "Running test_high_eccentricity..." << std::endl;
    
    OrbitalElements elements = {0};
    elements.epoch = 2460000.0;
    elements.mean_motion = 10.0;
    elements.eccentricity = 0.5;  // Moderately elliptical
    elements.inclination = 30.0 * DEG_TO_RAD;
    elements.raan = 0.0;
    elements.arg_perigee = 0.0;
    elements.mean_anomaly = 0.0;
    // Note: bstar, ndot, nddot are not part of the current OrbitalElements struct
    
    StateVectorECI state;
    int result = propagate(&elements, 0.0, &state);
    
    assert(result == PROPAGATION_SUCCESS);
    assert(is_valid_vector(state.r));
    assert(is_valid_vector(state.v));
    
    // For high eccentricity, the orbit should still be valid
    double r_mag = sqrt(state.r[0]*state.r[0] + state.r[1]*state.r[1] + state.r[2]*state.r[2]);
    assert(r_mag > 1000.0);  // Should be reasonable distance from Earth
    
    std::cout << "  High-e orbit position magnitude: " << r_mag << " km" << std::endl;
    std::cout << "  PASSED" << std::endl;
}

// Test case 5: J2 perturbation effects
void test_j2_effects() {
    std::cout << "Running test_j2_effects..." << std::endl;
    
    OrbitalElements elements = {0};
    elements.epoch = 2460000.0;
    elements.mean_motion = 14.0;
    elements.eccentricity = 0.001;
    elements.inclination = 90.0 * DEG_TO_RAD;  // Polar orbit - maximum J2 effect
    elements.raan = 0.0;
    elements.arg_perigee = 0.0;
    elements.mean_anomaly = 0.0;
    // Note: bstar, ndot, nddot are not part of the current OrbitalElements struct
    
    StateVectorECI state_short, state_long;
    
    // Compare short-term vs long-term propagation
    assert(propagate(&elements, 10.0, &state_short) == PROPAGATION_SUCCESS);
    assert(propagate(&elements, 1440.0, &state_long) == PROPAGATION_SUCCESS);  // 1 day
    
    assert(is_valid_vector(state_short.r) && is_valid_vector(state_short.v));
    assert(is_valid_vector(state_long.r) && is_valid_vector(state_long.v));
    
    // J2 effects should cause secular changes over time
    double dist = sqrt(pow(state_long.r[0] - state_short.r[0], 2) + 
                      pow(state_long.r[1] - state_short.r[1], 2) + 
                      pow(state_long.r[2] - state_short.r[2], 2));
    
    std::cout << "  Position difference after 1 day: " << dist << " km" << std::endl;
    std::cout << "  PASSED" << std::endl;
}

int main() {
    std::cout << "Starting propagation tests..." << std::endl << std::endl;
    
    try {
        test_basic_propagation();
        std::cout << std::endl;
        
        test_time_propagation();
        std::cout << std::endl;
        
        test_error_handling();
        std::cout << std::endl;
        
        test_high_eccentricity();
        std::cout << std::endl;
        
        test_j2_effects();
        std::cout << std::endl;
        
        std::cout << "All propagation tests PASSED!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
