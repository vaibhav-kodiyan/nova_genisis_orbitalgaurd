#include "maneuver.h"
#include "constants.h"
#include <cmath>
#include <cstdio>
#include <cassert>

// Test tolerances
#define FUEL_TOLERANCE 1e-6
#define VELOCITY_TOLERANCE 1e-9
#define POSITION_TOLERANCE 1e-9

// Helper function for floating point comparison
bool is_close(double a, double b, double tolerance) {
    return fabs(a - b) < tolerance;
}

// Test Group 1: Fuel consumption tests
void test_fuel_consumption() {
    printf("Testing fuel_consumption()...\n");
    
    // Test case 1: Small delta-v (10 m/s) - should use linear approximation
    double fuel1 = fuel_consumption(0.01, 300.0, 1000.0, 100.0, 1.0); // 10 m/s = 0.01 km/s
    double expected1 = 1100.0 * (10.0 / (300.0 * 9.80665)); // Linear approximation
    assert(is_close(fuel1, expected1, FUEL_TOLERANCE));
    printf("  ✓ Small delta-v (10 m/s): %.6f kg\n", fuel1);
    
    // Test case 2: Medium delta-v (100 m/s) - should use exponential
    double fuel2 = fuel_consumption(0.1, 300.0, 1000.0, 100.0, 1.0); // 100 m/s = 0.1 km/s
    double m0 = 1100.0;
    double mass_ratio = exp(-100.0 / (300.0 * 9.80665));
    double expected2 = m0 * (1.0 - mass_ratio);
    assert(is_close(fuel2, expected2, FUEL_TOLERANCE));
    printf("  ✓ Medium delta-v (100 m/s): %.6f kg\n", fuel2);
    
    // Test case 3: Large delta-v (1000 m/s) - exponential regime
    double fuel3 = fuel_consumption(1.0, 300.0, 1000.0, 100.0, 1.0); // 1000 m/s = 1.0 km/s
    double mass_ratio3 = exp(-1000.0 / (300.0 * 9.80665));
    double expected3 = m0 * (1.0 - mass_ratio3);
    assert(is_close(fuel3, expected3, FUEL_TOLERANCE));
    printf("  ✓ Large delta-v (1000 m/s): %.6f kg\n", fuel3);
    
    // Test case 4: Efficiency handling (50% efficiency)
    double fuel4 = fuel_consumption(0.1, 300.0, 1000.0, 100.0, 0.5); // 50% efficiency
    double eff_dv = 100.0 / 0.5; // Effective delta-v = 200 m/s
    double mass_ratio4 = exp(-eff_dv / (300.0 * 9.80665));
    double expected4 = m0 * (1.0 - mass_ratio4);
    assert(is_close(fuel4, expected4, FUEL_TOLERANCE));
    printf("  ✓ Efficiency test (50%%): %.6f kg\n", fuel4);
    
    // Test case 5: Zero/negative efficiency handling
    double fuel5 = fuel_consumption(0.1, 300.0, 1000.0, 100.0, 0.0); // Should default to 1.0
    assert(is_close(fuel5, fuel2, FUEL_TOLERANCE)); // Should equal 100% efficiency case
    printf("  ✓ Zero efficiency handling: %.6f kg\n", fuel5);
}

// Test Group 2: Plan avoidance success/failure tests
void test_plan_avoidance() {
    printf("Testing plan_avoidance()...\n");
    
    // Create test orbital elements (typical LEO satellite)
    OrbitalElements primary = {0};
    primary.semi_major_axis = 6800.0; // km
    primary.eccentricity = 0.001;
    primary.inclination = 0.785; // ~45 degrees
    primary.raan = 0.0;
    primary.arg_perigee = 0.0;
    primary.mean_anomaly = 0.0;
    primary.epoch = 2460000.0; // Julian date
    primary.mean_motion = 15.5; // rev/day
    
    OrbitalElements secondary = primary; // Same for simplicity
    
    Maneuver result_maneuver;
    
    // Test case 1: Success case - reasonable parameters
    double encounter_time = primary.epoch + 1.0; // 1 day later
    double target_distance = 5.0; // km
    double max_delta_v = 100.0; // m/s
    
    int result1 = plan_avoidance(&primary, &secondary, encounter_time, 
                                target_distance, max_delta_v, &result_maneuver);
    assert(result1 == 0); // Should succeed
    
    // Check that delta-v magnitude is reasonable
    double dv_mag = sqrt(result_maneuver.delta_v[0]*result_maneuver.delta_v[0] + 
                        result_maneuver.delta_v[1]*result_maneuver.delta_v[1] + 
                        result_maneuver.delta_v[2]*result_maneuver.delta_v[2]);
    assert(dv_mag <= max_delta_v);
    assert(dv_mag > 0.0);
    printf("  ✓ Success case: delta-v = %.3f m/s, ID = %s\n", dv_mag, result_maneuver.id);
    
    // Test case 2: Failure case - delta-v too large
    double small_max_dv = 0.001; // Very small limit
    int result2 = plan_avoidance(&primary, &secondary, encounter_time, 
                                target_distance, small_max_dv, &result_maneuver);
    assert(result2 != 0); // Should fail
    printf("  ✓ Failure case (delta-v too large): result = %d\n", result2);
    
    // Test case 3: Failure case - encounter in past
    double past_time = primary.epoch - 1.0; // 1 day in past
    int result3 = plan_avoidance(&primary, &secondary, past_time, 
                                target_distance, max_delta_v, &result_maneuver);
    assert(result3 != 0); // Should fail
    printf("  ✓ Failure case (past encounter): result = %d\n", result3);
    
    // Test case 4: Check deterministic ID generation
    Maneuver maneuver1, maneuver2;
    plan_avoidance(&primary, &secondary, encounter_time, target_distance, max_delta_v, &maneuver1);
    plan_avoidance(&primary, &secondary, encounter_time, target_distance, max_delta_v, &maneuver2);
    assert(strcmp(maneuver1.id, maneuver2.id) == 0); // IDs should be identical
    printf("  ✓ Deterministic ID: %s\n", maneuver1.id);
}

// Test Group 3: Apply maneuver tests
void test_apply_maneuver() {
    printf("Testing apply_maneuver()...\n");
    
    // Create test orbital elements
    OrbitalElements elements = {0};
    elements.semi_major_axis = 6800.0;
    elements.eccentricity = 0.001;
    elements.inclination = 0.785;
    elements.raan = 0.0;
    elements.arg_perigee = 0.0;
    elements.mean_anomaly = 0.0;
    elements.epoch = 2460000.0;
    elements.mean_motion = 15.5;
    
    // Create test maneuver
    Maneuver maneuver = {0};
    strcpy(maneuver.id, "TEST_001");
    maneuver.epoch = elements.epoch + 0.1; // 0.1 days later
    maneuver.delta_v[0] = 10.0; // m/s in X direction
    maneuver.delta_v[1] = 0.0;
    maneuver.delta_v[2] = 0.0;
    maneuver.fuel_cost = 1.0;
    
    // Get pre-maneuver state for comparison
    StateVectorECI pre_state;
    double minutes_offset = (maneuver.epoch - elements.epoch) * MINUTES_PER_DAY;
    int prop_result = propagate(&elements, minutes_offset, &pre_state);
    assert(prop_result == PROPAGATION_SUCCESS);
    
    // Apply maneuver
    StateVectorECI post_state;
    apply_maneuver(&elements, &maneuver, 0.0, &post_state);
    
    // Test case 1: Position should be unchanged at impulse instant
    assert(is_close(post_state.r[0], pre_state.r[0], POSITION_TOLERANCE));
    assert(is_close(post_state.r[1], pre_state.r[1], POSITION_TOLERANCE));
    assert(is_close(post_state.r[2], pre_state.r[2], POSITION_TOLERANCE));
    printf("  ✓ Position unchanged: [%.6f, %.6f, %.6f] km\n", 
           post_state.r[0], post_state.r[1], post_state.r[2]);
    
    // Test case 2: Velocity should be incremented by delta-v (converted to km/s)
    double expected_vx = pre_state.v[0] + (maneuver.delta_v[0] / 1000.0);
    double expected_vy = pre_state.v[1] + (maneuver.delta_v[1] / 1000.0);
    double expected_vz = pre_state.v[2] + (maneuver.delta_v[2] / 1000.0);
    
    assert(is_close(post_state.v[0], expected_vx, VELOCITY_TOLERANCE));
    assert(is_close(post_state.v[1], expected_vy, VELOCITY_TOLERANCE));
    assert(is_close(post_state.v[2], expected_vz, VELOCITY_TOLERANCE));
    printf("  ✓ Velocity incremented: [%.6f, %.6f, %.6f] km/s\n", 
           post_state.v[0], post_state.v[1], post_state.v[2]);
    
    // Test case 3: Time should be set to maneuver epoch
    assert(is_close(post_state.t, maneuver.epoch, 1e-10));
    printf("  ✓ Time set correctly: %.6f\n", post_state.t);
    
    // Test case 4: Basic propagation hookup verification
    // The fact that we got valid pre_state confirms propagation integration works
    printf("  ✓ Propagation hookup verified\n");
}

int main() {
    printf("=== Maneuver Module Tests ===\n\n");
    
    test_fuel_consumption();
    printf("\n");
    
    test_plan_avoidance();
    printf("\n");
    
    test_apply_maneuver();
    printf("\n");
    
    printf("=== All Tests Passed ===\n");
    return 0;
}
