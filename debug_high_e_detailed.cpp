#include <iostream>
#include <cmath>
#include "include/propagation.h"
#include "include/constants.h"

int main() {
    std::cout << "Testing high eccentricity orbit with detailed debugging..." << std::endl;
    
    OrbitalElements elements = {0};
    elements.epoch = 2460000.0;
    elements.mean_motion = 10.0;
    elements.eccentricity = 0.7;  // Highly elliptical
    elements.inclination = 30.0 * DEG_TO_RAD;
    elements.raan = 0.0;
    elements.arg_perigee = 0.0;
    elements.mean_anomaly = 0.0;
    elements.bstar = 0.0;
    elements.ndot = 0.0;
    elements.nddot = 0.0;
    
    std::cout << "Input parameters:" << std::endl;
    std::cout << "  Eccentricity: " << elements.eccentricity << std::endl;
    std::cout << "  Mean motion: " << elements.mean_motion << " rev/day" << std::endl;
    std::cout << "  Inclination: " << elements.inclination * RAD_TO_DEG << " deg" << std::endl;
    
    // Calculate expected semi-major axis
    double n_rad_per_sec = elements.mean_motion * TWO_PI / (24.0 * 3600.0);
    double expected_a = pow(MU / (n_rad_per_sec * n_rad_per_sec), 1.0/3.0);
    std::cout << "  Expected semi-major axis: " << expected_a << " km" << std::endl;
    std::cout << "  Expected perigee: " << expected_a * (1.0 - elements.eccentricity) << " km" << std::endl;
    std::cout << "  Expected apogee: " << expected_a * (1.0 + elements.eccentricity) << " km" << std::endl;
    
    StateVectorECI state;
    int result = propagate(&elements, 0.0, &state);
    
    std::cout << "\nPropagation result: " << result << std::endl;
    
    if (result == PROPAGATION_SUCCESS) {
        double r_mag = sqrt(state.r[0]*state.r[0] + state.r[1]*state.r[1] + state.r[2]*state.r[2]);
        std::cout << "Position magnitude: " << r_mag << " km" << std::endl;
        std::cout << "Position: [" << state.r[0] << ", " << state.r[1] << ", " << state.r[2] << "] km" << std::endl;
        std::cout << "Velocity: [" << state.v[0] << ", " << state.v[1] << ", " << state.v[2] << "] km/s" << std::endl;
        
        // Check for NaN individually
        for (int i = 0; i < 3; i++) {
            if (std::isnan(state.r[i])) std::cout << "  r[" << i << "] is NaN" << std::endl;
            if (std::isnan(state.v[i])) std::cout << "  v[" << i << "] is NaN" << std::endl;
        }
    } else {
        std::cout << "Error code: " << result << std::endl;
        if (result == PROPAGATION_ERROR_CONVERGENCE) std::cout << "Kepler solver failed to converge" << std::endl;
        if (result == PROPAGATION_ERROR_INVALID_INPUT) std::cout << "Invalid input parameters" << std::endl;
        if (result == PROPAGATION_ERROR_NAN_RESULT) std::cout << "NaN result detected" << std::endl;
    }
    
    return 0;
}
