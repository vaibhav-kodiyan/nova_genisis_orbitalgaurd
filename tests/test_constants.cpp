#include <cassert>
#include <cmath>
#include <iostream>
#include "../include/constants.h"

void test_gravitational_parameter() {
    // Verify MU matches expected value (km³/s²)
    assert(fabs(MU - 398600.4418) < 1e-4);
    assert(MU > 0);
}

void test_earth_radius() {
    // Verify Earth radius matches WGS-84 value (km)
    assert(fabs(EARTH_RADIUS - 6378.137) < 1e-3);
    assert(EARTH_RADIUS > 0);
}

void test_j2_coefficient() {
    // Verify J2 matches expected value
    assert(fabs(J2 - 0.00108262998905) < 1e-14);
    assert(J2 > 0);
}

void test_mathematical_constants() {
    // Verify PI and TWO_PI
    assert(fabs(PI - M_PI) < 1e-15);
    assert(fabs(TWO_PI - 2.0 * M_PI) < 1e-15);
    assert(PI > 0);
    assert(TWO_PI > 0);
}

void test_conversion_factors() {
    // Verify degree/radian conversions
    assert(fabs(DEG_TO_RAD - M_PI / 180.0) < 1e-15);
    assert(fabs(RAD_TO_DEG - 180.0 / M_PI) < 1e-15);
    
    // Test conversion consistency
    double test_angle = 45.0;
    double rad_result = test_angle * DEG_TO_RAD * RAD_TO_DEG;
    assert(fabs(rad_result - test_angle) < 1e-12);
}

int main() {
    test_gravitational_parameter();
    test_earth_radius();
    test_j2_coefficient();
    test_mathematical_constants();
    test_conversion_factors();
    
    std::cout << "All constant tests passed!" << std::endl;
    return 0;
}
