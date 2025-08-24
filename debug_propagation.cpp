#include <iostream>
#include <cmath>
#include "include/propagation.h"
#include "include/constants.h"

int main() {
    std::cout << "Starting debug program..." << std::endl;
    std::cout << "MU = " << MU << std::endl;
    std::cout << "EARTH_RADIUS = " << EARTH_RADIUS << std::endl;
    
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
    
    std::cout << "Input orbital elements:" << std::endl;
    std::cout << "  Mean motion: " << elements.mean_motion << " rev/day" << std::endl;
    std::cout << "  Eccentricity: " << elements.eccentricity << std::endl;
    std::cout << "  Inclination: " << elements.inclination * RAD_TO_DEG << " deg" << std::endl;
    
    // Calculate expected semi-major axis
    double n_rad_per_sec = elements.mean_motion * TWO_PI / (24.0 * 3600.0);
    double expected_a = pow(MU / (n_rad_per_sec * n_rad_per_sec), 1.0/3.0);
    std::cout << "  Expected semi-major axis: " << expected_a << " km" << std::endl;
    std::cout << "  Expected altitude: " << expected_a - EARTH_RADIUS << " km" << std::endl;
    
    StateVectorECI state;
    
    // Debug intermediate calculations
    double n_rad_per_min = elements.mean_motion * TWO_PI / 1440.0;
    double calc_a = pow(MU / (n_rad_per_min * n_rad_per_min), 1.0/3.0);
    std::cout << "  Calculated semi-major axis: " << calc_a << " km" << std::endl;
    
    int result = propagate(&elements, 0.0, &state);
    
    std::cout << "\nPropagation result: " << result << std::endl;
    
    if (result == PROPAGATION_SUCCESS) {
        std::cout << "Position: [" << state.r[0] << ", " << state.r[1] << ", " << state.r[2] << "] km" << std::endl;
        std::cout << "Velocity: [" << state.v[0] << ", " << state.v[1] << ", " << state.v[2] << "] km/s" << std::endl;
        
        double r_mag = sqrt(state.r[0]*state.r[0] + state.r[1]*state.r[1] + state.r[2]*state.r[2]);
        double v_mag = sqrt(state.v[0]*state.v[0] + state.v[1]*state.v[1] + state.v[2]*state.v[2]);
        
        std::cout << "Position magnitude: " << r_mag << " km" << std::endl;
        std::cout << "Velocity magnitude: " << v_mag << " km/s" << std::endl;
        std::cout << "Altitude: " << r_mag - EARTH_RADIUS << " km" << std::endl;
    } else {
        std::cout << "Propagation failed with error code: " << result << std::endl;
    }
    
    return 0;
}
