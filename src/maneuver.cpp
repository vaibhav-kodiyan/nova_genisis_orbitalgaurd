#include "maneuver.h"
#include "constants.h"
#include "propagation.h"
#include <cmath>
#include <cstring>
#include <cstdio>

// Standard gravitational acceleration (m/s²)
#define G0 9.80665

// Tolerance for small delta-v approximation
#define SMALL_DV_THRESHOLD 1e-3

// Helper function to compute vector magnitude
static double vector_magnitude(const double v[3]) {
    return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

// Helper function to normalize a vector
static void normalize_vector(const double v[3], double normalized[3]) {
    double mag = vector_magnitude(v);
    if (mag > 1e-15) {
        normalized[0] = v[0] / mag;
        normalized[1] = v[1] / mag;
        normalized[2] = v[2] / mag;
    } else {
        normalized[0] = normalized[1] = normalized[2] = 0.0;
    }
}

double fuel_consumption(double delta_v_km_s, double specific_impulse_s, 
                       double dry_mass_kg, double propellant_mass_kg, double efficiency) {
    // Input validation and efficiency handling
    if (efficiency <= 0.0) {
        efficiency = 1.0; // Default to perfect efficiency, log comment: treat as 1.0
    }
    
    // Convert delta_v from km/s to m/s
    double delta_v_ms = delta_v_km_s * 1000.0;
    
    // Apply efficiency: effective delta-v = delta_v / efficiency
    double dv_eff = delta_v_ms / efficiency;
    
    // Total initial mass
    double m0 = dry_mass_kg + propellant_mass_kg;
    
    // Check for small delta-v approximation
    double dv_ratio = dv_eff / (specific_impulse_s * G0);
    
    if (dv_ratio < SMALL_DV_THRESHOLD) {
        // Linear approximation: required prop mass fraction ≈ Δv / (Isp * g0)
        return m0 * dv_ratio;
    } else {
        // Full rocket equation: Δv = Isp * g0 * ln(m0/m1)
        // Solving for m1: m1 = m0 * exp(-Δv/(Isp*g0))
        double mass_ratio = exp(-dv_eff / (specific_impulse_s * G0));
        double final_mass = m0 * mass_ratio;
        return m0 - final_mass; // Required fuel mass
    }
}

int plan_avoidance(const OrbitalElements* primary, const OrbitalElements* secondary,
                  double encounter_time, double target_distance_km, 
                  double max_delta_v_mps, Maneuver* out_maneuver) {
    // TODO: Implementation placeholder
    return -1;
}

void apply_maneuver(const OrbitalElements* elements, const Maneuver* m,
                   double current_unix_ms, StateVectorECI* out_state) {
    // TODO: Implementation placeholder
}

double isp_default_chemical() {
    return 300.0; // seconds
}

double isp_electric() {
    return 3000.0; // seconds
}
