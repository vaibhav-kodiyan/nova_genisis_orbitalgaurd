#include <iostream>
#include <cassert>
#include <cmath>
#include <cstring>
#include "../include/screening.h"
#include "../include/constants.h"

// Test helper function to create a state vector
StateVectorECI create_state_vector(double t, double x, double y, double z, double vx, double vy, double vz) {
    StateVectorECI state;
    state.t = t;
    state.r[0] = x; state.r[1] = y; state.r[2] = z;
    state.v[0] = vx; state.v[1] = vy; state.v[2] = vz;
    return state;
}

void test_distance3d() {
    std::cout << "Testing distance3d()..." << std::endl;
    
    // Test basic distance calculation
    double pos1[3] = {0.0, 0.0, 0.0};
    double pos2[3] = {3.0, 4.0, 0.0};
    double dist = distance3d(pos1, pos2);
    assert(fabs(dist - 5.0) < 1e-10);
    
    // Test 3D distance
    double pos3[3] = {1.0, 1.0, 1.0};
    double pos4[3] = {4.0, 5.0, 1.0};
    dist = distance3d(pos3, pos4);
    assert(fabs(dist - 5.0) < 1e-10);
    
    // Test null pointer handling
    dist = distance3d(nullptr, pos2);
    assert(dist == -1.0);
    
    std::cout << "✓ distance3d() tests passed" << std::endl;
}

void test_classify_severity() {
    std::cout << "Testing classify_severity()..." << std::endl;
    
    // Test severity classifications
    assert(classify_severity(0.5) == CRASH);
    assert(classify_severity(1.0) == CRASH);
    assert(classify_severity(3.0) == HIGH);
    assert(classify_severity(5.0) == HIGH);
    assert(classify_severity(15.0) == MEDIUM);
    assert(classify_severity(25.0) == MEDIUM);
    assert(classify_severity(50.0) == LOW);
    
    // Test edge cases
    assert(classify_severity(-1.0) == NONE);
    
    std::cout << "✓ classify_severity() tests passed" << std::endl;
}

void test_logistic_probability() {
    std::cout << "Testing logistic_probability()..." << std::endl;
    
    // Test probability function behavior
    double prob1 = logistic_probability(1.0, 5.0);
    double prob2 = logistic_probability(10.0, 5.0);
    double prob3 = logistic_probability(5.0, 5.0);
    
    // Closer distances should have higher probability
    assert(prob1 > prob2);
    
    // Probability should be between 0 and 1
    assert(prob1 >= 0.0 && prob1 <= 1.0);
    assert(prob2 >= 0.0 && prob2 <= 1.0);
    assert(prob3 >= 0.0 && prob3 <= 1.0);
    
    // Test edge cases
    assert(logistic_probability(-1.0, 5.0) == 0.0);
    assert(logistic_probability(5.0, -1.0) == 0.0);
    
    std::cout << "✓ logistic_probability() tests passed" << std::endl;
}

void test_compute_relative_velocity() {
    std::cout << "Testing compute_relative_velocity()..." << std::endl;
    
    StateVectorECI state1 = create_state_vector(0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    StateVectorECI state2 = create_state_vector(0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    
    double rel_vel = compute_relative_velocity(&state1, &state2);
    assert(fabs(rel_vel - sqrt(2.0)) < 1e-10);
    
    // Test null pointer handling
    rel_vel = compute_relative_velocity(nullptr, &state2);
    assert(rel_vel == 0.0);
    
    std::cout << "✓ compute_relative_velocity() tests passed" << std::endl;
}

void test_screen_conjunctions() {
    std::cout << "Testing screen_conjunctions()..." << std::endl;
    
    // Create synthetic trajectories for two satellites with a known close approach
    const int NUM_STEPS = 10;
    StateVectorECI traj1[NUM_STEPS];
    StateVectorECI traj2[NUM_STEPS];
    
    // Satellite 1: moving in +X direction
    for (int i = 0; i < NUM_STEPS; i++) {
        traj1[i] = create_state_vector(
            i * 0.1,  // time
            i * 1.0, 0.0, 0.0,  // position (moving in X)
            1.0, 0.0, 0.0       // velocity
        );
    }
    
    // Satellite 2: moving in +Y direction, intersecting at origin
    for (int i = 0; i < NUM_STEPS; i++) {
        traj2[i] = create_state_vector(
            i * 0.1,  // time
            0.0, i * 1.0 - 5.0, 0.0,  // position (moving in Y, offset to intersect)
            0.0, 1.0, 0.0              // velocity
        );
    }
    
    const StateVectorECI* trajectories[2] = {traj1, traj2};
    const char* ids[2] = {"SAT1", "SAT2"};
    
    Encounter encounters[10];
    size_t encounter_count;
    
    int result = screen_conjunctions(trajectories, ids, 2, 10.0, encounters, &encounter_count, 10);
    
    assert(result == SCREENING_SUCCESS);
    assert(encounter_count > 0);
    
    // Verify encounter details
    if (encounter_count > 0) {
        assert(strcmp(encounters[0].sat1_id, "SAT1") == 0);
        assert(strcmp(encounters[0].sat2_id, "SAT2") == 0);
        assert(encounters[0].min_distance_km >= 0.0);
        assert(encounters[0].collision_probability >= 0.0 && encounters[0].collision_probability <= 1.0);
    }
    
    // Test error conditions
    result = screen_conjunctions(nullptr, ids, 2, 10.0, encounters, &encounter_count, 10);
    assert(result == SCREENING_ERROR_INVALID_INPUT);
    
    std::cout << "✓ screen_conjunctions() tests passed" << std::endl;
}

void test_sorting_and_filtering() {
    std::cout << "Testing sorting and filtering functions..." << std::endl;
    
    // Create test encounters
    Encounter encounters[3];
    
    // Initialize encounters with different risk levels
    strcpy(encounters[0].sat1_id, "SAT1");
    strcpy(encounters[0].sat2_id, "SAT2");
    encounters[0].time_of_closest_approach = 2.0;
    encounters[0].collision_probability = 0.3;
    encounters[0].severity = MEDIUM;
    
    strcpy(encounters[1].sat1_id, "SAT3");
    strcpy(encounters[1].sat2_id, "SAT4");
    encounters[1].time_of_closest_approach = 1.0;
    encounters[1].collision_probability = 0.8;
    encounters[1].severity = HIGH;
    
    strcpy(encounters[2].sat1_id, "SAT5");
    strcpy(encounters[2].sat2_id, "SAT6");
    encounters[2].time_of_closest_approach = 3.0;
    encounters[2].collision_probability = 0.1;
    encounters[2].severity = LOW;
    
    // Test sorting by time
    sort_encounters_by_time(encounters, 3);
    assert(encounters[0].time_of_closest_approach <= encounters[1].time_of_closest_approach);
    assert(encounters[1].time_of_closest_approach <= encounters[2].time_of_closest_approach);
    
    // Test sorting by risk
    sort_encounters_by_risk(encounters, 3);
    double risk0 = encounters[0].collision_probability * (encounters[0].severity + 1);
    double risk1 = encounters[1].collision_probability * (encounters[1].severity + 1);
    assert(risk0 >= risk1);
    
    // Test filtering by probability
    size_t filtered_count = filter_by_probability(encounters, 3, 0.2);
    assert(filtered_count <= 3);
    
    // All remaining encounters should have probability >= 0.2
    for (size_t i = 0; i < filtered_count; i++) {
        assert(encounters[i].collision_probability >= 0.2);
    }
    
    std::cout << "✓ Sorting and filtering tests passed" << std::endl;
}

int main() {
    std::cout << "Running screening module tests..." << std::endl;
    
    test_distance3d();
    test_classify_severity();
    test_logistic_probability();
    test_compute_relative_velocity();
    test_screen_conjunctions();
    test_sorting_and_filtering();
    
    std::cout << "\n✅ All screening tests passed successfully!" << std::endl;
    return 0;
}
