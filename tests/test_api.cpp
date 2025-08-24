#include "api.h"
#include "propagation.h"
#include "types.h"
#include "time_utils.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <cmath>

// Test data - ISS TLE from 2023
const char* ISS_NAME = "ISS (ZARYA)";
const char* ISS_LINE1 = "1 25544U 98067A   23001.00000000  .00002182  00000-0  40768-4 0  9992";
const char* ISS_LINE2 = "2 25544  51.6461 339.2971 0002972  68.7102 291.5211 15.48919103123456";

void test_tle_parsing() {
    std::cout << "Running test_tle_parsing..." << std::endl;
    
    // Test valid TLE parsing
    void* elements = og_parse_tle(ISS_NAME, ISS_LINE1, ISS_LINE2);
    assert(elements != nullptr);
    assert(og_last_error() == nullptr);
    
    og_free_elements(elements);
    assert(og_last_error() == nullptr);
    
    // Test invalid TLE parsing
    void* invalid_elements = og_parse_tle(ISS_NAME, "invalid", "invalid");
    assert(invalid_elements == nullptr);
    assert(og_last_error() != nullptr);
    
    // Test null inputs
    void* null_elements = og_parse_tle(nullptr, ISS_LINE1, ISS_LINE2);
    assert(null_elements == nullptr);
    assert(og_last_error() != nullptr);
    
    std::cout << "✓ TLE parsing tests passed" << std::endl;
}

void test_propagation_parity() {
    std::cout << "Running test_propagation_parity..." << std::endl;
    
    // Parse TLE using API
    void* api_elements = og_parse_tle(ISS_NAME, ISS_LINE1, ISS_LINE2);
    assert(api_elements != nullptr);
    
    // Create equivalent internal elements for comparison
    OrbitalElements internal_elements = {0};
    internal_elements.epoch = 2460001.5; // Approximate Julian date for 2023-01-01
    internal_elements.mean_motion = 15.48919103;
    internal_elements.eccentricity = 0.0002972;
    internal_elements.inclination = 51.6461 * (3.14159265358979323846 / 180.0);
    internal_elements.raan = 339.2971 * (3.14159265358979323846 / 180.0);
    internal_elements.arg_perigee = 68.7102 * (3.14159265358979323846 / 180.0);
    internal_elements.mean_anomaly = 291.5211 * (3.14159265358979323846 / 180.0);
    
    // Calculate semi-major axis
    double n_rad_per_sec = internal_elements.mean_motion * 2.0 * 3.14159265358979323846 / (24.0 * 3600.0);
    internal_elements.semi_major_axis = pow(398600.4418 / (n_rad_per_sec * n_rad_per_sec), 1.0/3.0);
    
    // Set aliases
    internal_elements.tilt = internal_elements.inclination;
    internal_elements.node = internal_elements.raan;
    internal_elements.perigee_angle = internal_elements.arg_perigee;
    internal_elements.position = internal_elements.mean_anomaly;
    internal_elements.time = internal_elements.epoch;
    
    // Propagate using both API and internal function
    double api_pos[3], api_vel[3];
    int api_result = og_propagate(api_elements, 90.0, api_pos, api_vel); // 90 minutes
    assert(api_result == 0);
    
    StateVectorECI internal_state;
    int internal_result = propagate(&internal_elements, 90.0, &internal_state);
    assert(internal_result == 0);
    
    // Compare results (allow for some tolerance due to TLE parsing differences)
    double pos_diff = sqrt(pow(api_pos[0] - internal_state.r[0], 2) +
                          pow(api_pos[1] - internal_state.r[1], 2) +
                          pow(api_pos[2] - internal_state.r[2], 2));
    
    double vel_diff = sqrt(pow(api_vel[0] - internal_state.v[0], 2) +
                          pow(api_vel[1] - internal_state.v[1], 2) +
                          pow(api_vel[2] - internal_state.v[2], 2));
    
    std::cout << "Position difference: " << pos_diff << " km" << std::endl;
    std::cout << "Velocity difference: " << vel_diff << " km/s" << std::endl;
    
    // Results should be reasonably close (within 100 km and 0.1 km/s)
    assert(pos_diff < 100.0);
    assert(vel_diff < 0.1);
    
    og_free_elements(api_elements);
    
    std::cout << "✓ Propagation parity test passed" << std::endl;
}

void test_screening_smoke() {
    std::cout << "Running test_screening_smoke..." << std::endl;
    
    // Create test satellite positions
    double sat1_pos[3] = {6800.0, 0.0, 0.0};
    double sat2_pos[3] = {6801.0, 0.0, 0.0}; // 1 km apart
    double sat3_pos[3] = {6850.0, 0.0, 0.0}; // 50 km apart
    
    const double* states[3] = {sat1_pos, sat2_pos, sat3_pos};
    const char* ids[3] = {"SAT1", "SAT2", "SAT3"};
    
    og_encounter_t encounters[10];
    
    // Test with 5 km threshold - should find 1 encounter (SAT1-SAT2)
    size_t count = og_screen(states, ids, 3, 5.0, encounters, 10);
    assert(count == 1);
    assert(encounters[0].min_distance_km < 5.0);
    assert(strcmp(encounters[0].id_a, "SAT1") == 0);
    assert(strcmp(encounters[0].id_b, "SAT2") == 0);
    
    // Check that TCA time is set (should be current time)
    assert(encounters[0].tca_time.year > 2020);
    
    // Test with 100 km threshold - should find 3 encounters (all pairs)
    count = og_screen(states, ids, 3, 100.0, encounters, 10);
    assert(count == 3);
    
    // Test with 0.5 km threshold - should find 0 encounters
    count = og_screen(states, ids, 3, 0.5, encounters, 10);
    assert(count == 0);
    
    std::cout << "✓ Screening smoke test passed" << std::endl;
}

void test_maneuver_sanity() {
    std::cout << "Running test_maneuver_sanity..." << std::endl;
    
    // Create two satellite elements
    void* primary = og_parse_tle("PRIMARY", ISS_LINE1, ISS_LINE2);
    void* secondary = og_parse_tle("SECONDARY", ISS_LINE1, ISS_LINE2);
    assert(primary != nullptr && secondary != nullptr);
    
    og_maneuver_t maneuver;
    
    // Create encounter time (January 2, 2023)
    gregorian_time_t encounter_time = {2023, 1, 2, 12, 0, 0.0};
    
    // Test maneuver planning
    int result = og_plan_maneuver(primary, secondary, &encounter_time, 10.0, 100.0, &maneuver);
    assert(result == 0);
    
    // Check that maneuver structure is filled
    assert(maneuver.time.year == 2023);
    assert(maneuver.time.month == 1);
    assert(maneuver.time.day == 2);
    assert(strlen(maneuver.id) > 0);
    
    // Delta-V magnitude should be reasonable
    double dv_mag = sqrt(maneuver.delta_v[0]*maneuver.delta_v[0] + 
                        maneuver.delta_v[1]*maneuver.delta_v[1] + 
                        maneuver.delta_v[2]*maneuver.delta_v[2]);
    assert(dv_mag >= 0 && dv_mag <= 100.0); // Within specified limit
    
    og_free_elements(primary);
    og_free_elements(secondary);
    
    std::cout << "✓ Maneuver sanity test passed" << std::endl;
}

void test_fuel_consumption() {
    std::cout << "Running test_fuel_consumption..." << std::endl;
    
    // Test valid fuel calculation
    double fuel = og_fuel_consumption(0.1, 300.0, 1000.0, 200.0, 0.9);
    assert(fuel > 0 && fuel <= 200.0);
    
    // Test insufficient propellant case
    fuel = og_fuel_consumption(10.0, 300.0, 1000.0, 10.0, 0.9);
    assert(fuel < 0); // Should return -1 for insufficient propellant
    
    // Test invalid parameters
    fuel = og_fuel_consumption(-1.0, 300.0, 1000.0, 200.0, 0.9);
    assert(fuel < 0);
    assert(og_last_error() != nullptr);
    
    std::cout << "✓ Fuel consumption test passed" << std::endl;
}

void test_lifecycle() {
    std::cout << "Running test_lifecycle..." << std::endl;
    
    // Test multiple allocation/free cycles
    for (int i = 0; i < 10; i++) {
        void* elements = og_parse_tle(ISS_NAME, ISS_LINE1, ISS_LINE2);
        assert(elements != nullptr);
        og_free_elements(elements);
    }
    
    // Test freeing null pointer (should not crash)
    og_free_elements(nullptr);
    
    std::cout << "✓ Lifecycle test passed" << std::endl;
}

void test_time_utilities() {
    std::cout << "Running test_time_utilities..." << std::endl;
    
    // Test Gregorian to Julian conversion
    gregorian_time_t greg = {2023, 1, 1, 12, 0, 0.0};
    double jd = gregorian_to_julian(&greg);
    assert(jd > 2459000.0 && jd < 2461000.0); // Reasonable Julian date for 2023
    
    // Test Julian to Gregorian conversion (round trip)
    gregorian_time_t greg_back;
    julian_to_gregorian(jd, &greg_back);
    assert(greg_back.year == 2023);
    assert(greg_back.month == 1);
    assert(greg_back.day == 1);
    assert(greg_back.hour == 12);
    
    // Test ISO 8601 parsing
    gregorian_time_t parsed_time;
    int result = parse_iso8601_to_gregorian("2023-01-01T12:00:00", &parsed_time);
    assert(result == 0);
    assert(parsed_time.year == 2023);
    assert(parsed_time.month == 1);
    assert(parsed_time.day == 1);
    assert(parsed_time.hour == 12);
    
    // Test ISO 8601 formatting
    char buffer[64];
    int len = format_gregorian_to_iso8601(&greg, buffer, sizeof(buffer));
    assert(len > 0);
    assert(strncmp(buffer, "2023-01-01T12:00:00", 19) == 0);
    
    std::cout << "✓ Time utilities test passed" << std::endl;
}

int main() {
    std::cout << "Running API tests..." << std::endl;
    
    try {
        test_time_utilities();
        test_tle_parsing();
        test_propagation_parity();
        test_screening_smoke();
        test_maneuver_sanity();
        test_fuel_consumption();
        test_lifecycle();
        
        std::cout << "✅ All API tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}
