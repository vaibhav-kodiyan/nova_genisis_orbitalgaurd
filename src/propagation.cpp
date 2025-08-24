#include "propagation.h"
#include "constants.h"
#include <cmath>
#include <cstring>

// Tolerance and maximum iterations for Kepler equation solver
#define KEPLER_TOLERANCE 1e-10
#define KEPLER_MAX_ITERATIONS 30

// Minutes per day conversion
#define MINUTES_PER_DAY 1440.0

static double solve_kepler(double M, double e) {
    // Wrap mean anomaly to [0, 2π]
    M = fmod(M + TWO_PI, TWO_PI);
    
    // Better initial guess for high eccentricity
    double E;
    if (e < 0.8) {
        E = M + e * sin(M);  // Standard initial guess
    } else {
        // For high eccentricity, use Danby's method for initial guess
        E = M + 0.85 * e * ((sin(M) >= 0.0) ? 1.0 : -1.0);
    }
    
    // Newton-Raphson iteration with improved convergence
    for (int i = 0; i < KEPLER_MAX_ITERATIONS; i++) {
        double sin_E = sin(E);
        double cos_E = cos(E);
        double f = E - e * sin_E - M;
        double df = 1.0 - e * cos_E;
        
        // Check for zero derivative
        if (fabs(df) < 1e-15) {
            return NAN;
        }
        
        double delta = f / df;
        E -= delta;
        
        // Check for convergence
        if (fabs(delta) < KEPLER_TOLERANCE) {
            return E;
        }
    }
    
    // Failed to converge
    return NAN;
}

static void compute_pqw_to_eci_rotation(double raan, double inc, double argp, double rotation_matrix[9]) {
    double cos_raan = cos(raan);
    double sin_raan = sin(raan);
    double cos_inc = cos(inc);
    double sin_inc = sin(inc);
    double cos_argp = cos(argp);
    double sin_argp = sin(argp);
    
    // Rotation matrix from PQW to ECI coordinates
    // R = R3(-RAAN) * R1(-inc) * R3(-argp)
    rotation_matrix[0] = cos_raan * cos_argp - sin_raan * sin_argp * cos_inc;
    rotation_matrix[1] = -cos_raan * sin_argp - sin_raan * cos_argp * cos_inc;
    rotation_matrix[2] = sin_raan * sin_inc;
    
    rotation_matrix[3] = sin_raan * cos_argp + cos_raan * sin_argp * cos_inc;
    rotation_matrix[4] = -sin_raan * sin_argp + cos_raan * cos_argp * cos_inc;
    rotation_matrix[5] = -cos_raan * sin_inc;
    
    rotation_matrix[6] = sin_argp * sin_inc;
    rotation_matrix[7] = cos_argp * sin_inc;
    rotation_matrix[8] = cos_inc;
}

static void matrix_vector_multiply(const double matrix[9], const double input[3], double output[3]) {
    output[0] = matrix[0] * input[0] + matrix[1] * input[1] + matrix[2] * input[2];
    output[1] = matrix[3] * input[0] + matrix[4] * input[1] + matrix[5] * input[2];
    output[2] = matrix[6] * input[0] + matrix[7] * input[1] + matrix[8] * input[2];
}

static void apply_j2_corrections(const OrbitalElements* elements, OrbitalElements* corrected_elements, double minutes_since_epoch) {
    // Copy original elements
    memcpy(corrected_elements, elements, sizeof(OrbitalElements));
    
    // Convert mean motion from rev/day to rad/min
    double n0 = elements->mean_motion * TWO_PI / MINUTES_PER_DAY;
    
    // Semi-major axis (km)
    double a0 = pow(MU / (n0 * n0), 1.0/3.0);
    
    // J2 perturbation calculations
    double cos_inc = cos(elements->inclination);
    double sin_inc = sin(elements->inclination);
    double cos_inc_sq = cos_inc * cos_inc;
    
    // J2 correction factors
    double temp = 1.5 * J2 * (EARTH_RADIUS * EARTH_RADIUS) / (a0 * a0);
    double del1 = temp * (3.0 * cos_inc_sq - 1.0) / pow(1.0 - elements->eccentricity * elements->eccentricity, 1.5);
    double a1 = a0 * (1.0 - del1 / 3.0 - del1 * del1 - 134.0 * del1 * del1 * del1 / 81.0);
    
    // Corrected mean motion
    double n1 = sqrt(MU / (a1 * a1 * a1));
    
    // Secular rates due to J2
    double delta_omega = -temp * cos_inc * n1;  // RAAN rate
    double delta_w = temp * (5.0 * cos_inc_sq - 1.0) * n1 / 2.0;  // Argument of perigee rate
    
    // Apply secular corrections
    double dt = minutes_since_epoch;
    corrected_elements->raan += delta_omega * dt;
    corrected_elements->arg_perigee += delta_w * dt;
    corrected_elements->mean_motion = n1 * MINUTES_PER_DAY / TWO_PI;  // Convert back to rev/day
    
    // Wrap angles to [0, 2π]
    corrected_elements->raan = fmod(corrected_elements->raan + TWO_PI, TWO_PI);
    corrected_elements->arg_perigee = fmod(corrected_elements->arg_perigee + TWO_PI, TWO_PI);
}

int propagate(const OrbitalElements* elements, double minutes_since_epoch, StateVectorECI* out_state) {
    // Input validation
    if (!elements || !out_state) {
        return PROPAGATION_ERROR_INVALID_INPUT;
    }
    
    // Check for invalid eccentricity
    if (elements->eccentricity < 0.0 || elements->eccentricity >= 1.0) {
        return PROPAGATION_ERROR_INVALID_INPUT;
    }
    
    // Initialize output state to zeros
    memset(out_state, 0, sizeof(StateVectorECI));
    out_state->t = elements->epoch + minutes_since_epoch / MINUTES_PER_DAY;
    
    // Apply J2 corrections
    OrbitalElements corrected_elements;
    apply_j2_corrections(elements, &corrected_elements, minutes_since_epoch);
    
    // Convert mean motion from rev/day to rad/s
    double n_rad_per_sec = corrected_elements.mean_motion * TWO_PI / (24.0 * 3600.0);
    
    // Compute semi-major axis
    double a = pow(MU / (n_rad_per_sec * n_rad_per_sec), 1.0/3.0);
    
    // Convert mean motion to rad/min for time calculations
    double n = corrected_elements.mean_motion * TWO_PI / MINUTES_PER_DAY;
    
    // Compute mean anomaly at the specified time
    double M = corrected_elements.mean_anomaly + n * minutes_since_epoch;
    M = fmod(M + TWO_PI, TWO_PI);  // Wrap to [0, 2π]
    
    // Solve Kepler's equation for eccentric anomaly
    double E = solve_kepler(M, corrected_elements.eccentricity);
    if (std::isnan(E)) {
        return PROPAGATION_ERROR_CONVERGENCE;
    }
    
    // Compute true anomaly
    double cos_E = cos(E);
    double sin_E = sin(E);
    double beta = sqrt(1.0 - corrected_elements.eccentricity * corrected_elements.eccentricity);
    double cos_nu = (cos_E - corrected_elements.eccentricity) / (1.0 - corrected_elements.eccentricity * cos_E);
    double sin_nu = (beta * sin_E) / (1.0 - corrected_elements.eccentricity * cos_E);
    
    // Distance from focus
    double r_mag = a * (1.0 - corrected_elements.eccentricity * cos_E);
    
    // Position in PQW coordinates
    double r_pqw[3];
    r_pqw[0] = r_mag * cos_nu;
    r_pqw[1] = r_mag * sin_nu;
    r_pqw[2] = 0.0;
    
    // Velocity in PQW coordinates using standard orbital mechanics
    double v_pqw[3];
    double p = a * (1.0 - corrected_elements.eccentricity * corrected_elements.eccentricity);  // Semi-latus rectum
    double sqrt_mu_over_p = sqrt(MU / p);
    
    v_pqw[0] = -sqrt_mu_over_p * sin_nu;
    v_pqw[1] = sqrt_mu_over_p * (corrected_elements.eccentricity + cos_nu);
    v_pqw[2] = 0.0;
    
    // Compute rotation matrix from PQW to ECI
    double rotation_matrix[9];
    compute_pqw_to_eci_rotation(corrected_elements.raan, corrected_elements.inclination, corrected_elements.arg_perigee, rotation_matrix);
    
    // Transform position and velocity to ECI coordinates
    matrix_vector_multiply(rotation_matrix, r_pqw, out_state->r);
    matrix_vector_multiply(rotation_matrix, v_pqw, out_state->v);
    
    // Check for NaN results
    for (int i = 0; i < 3; i++) {
        if (std::isnan(out_state->r[i]) || std::isnan(out_state->v[i])) {
            memset(out_state, 0, sizeof(StateVectorECI));
            return PROPAGATION_ERROR_NAN_RESULT;
        }
    }
    
    return PROPAGATION_SUCCESS;
}
