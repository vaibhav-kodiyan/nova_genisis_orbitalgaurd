#include <iostream>
#include <cmath>
#include "include/propagation.h"
#include "include/constants.h"

int main() {
    std::cout << "Testing high eccentricity orbit..." << std::endl;
    
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
    
    std::cout << "Eccentricity: " << elements.eccentricity << std::endl;
    std::cout << "Mean motion: " << elements.mean_motion << " rev/day" << std::endl;
    
    StateVectorECI state;
    int result = propagate(&elements, 0.0, &state);
    
    std::cout << "Propagation result: " << result << std::endl;
    
    if (result == PROPAGATION_SUCCESS) {
        double r_mag = sqrt(state.r[0]*state.r[0] + state.r[1]*state.r[1] + state.r[2]*state.r[2]);
        std::cout << "Position magnitude: " << r_mag << " km" << std::endl;
        std::cout << "Position: [" << state.r[0] << ", " << state.r[1] << ", " << state.r[2] << "] km" << std::endl;
    } else {
        std::cout << "Error code: " << result << std::endl;
        if (result == PROPAGATION_ERROR_CONVERGENCE) std::cout << "Kepler solver failed to converge" << std::endl;
        if (result == PROPAGATION_ERROR_INVALID_INPUT) std::cout << "Invalid input parameters" << std::endl;
        if (result == PROPAGATION_ERROR_NAN_RESULT) std::cout << "NaN result detected" << std::endl;
    }
    
    return 0;
}
