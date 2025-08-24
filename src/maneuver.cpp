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
    // Input validation
    if (!primary || !out_maneuver) {
        return -1;
    }
    
    // Initialize output maneuver
    memset(out_maneuver, 0, sizeof(Maneuver));
    
    // Compute time difference (encounter_time is in Julian date, same as repo timebase)
    double current_time = primary->epoch; // Use primary's epoch as reference
    double delta_t_days = encounter_time - current_time;
    double delta_t_seconds = delta_t_days * SECONDS_PER_DAY;
    
    // Check if encounter time is in the past
    if (delta_t_seconds <= 0.0) {
        return -1; // Cannot plan maneuver for past encounter
    }
    
    // Heuristic: along-track displacement ≈ Δv * Δt
    // Required scalar Δv ≈ target_distance_km * 1000 / Δt (m/s)
    double required_dv_ms = (target_distance_km * 1000.0) / delta_t_seconds;
    
    // Check if required delta-v exceeds maximum
    if (required_dv_ms > max_delta_v_mps) {
        return -1; // Required delta-v too large
    }
    
    // Get current state vector to determine velocity direction
    StateVectorECI current_state;
    int prop_result = propagate(primary, 0.0, &current_state); // Propagate to current epoch
    if (prop_result != PROPAGATION_SUCCESS) {
        return -1; // Propagation failed
    }
    
    // Normalize velocity vector to get along-track direction
    double velocity_unit[3];
    normalize_vector(current_state.v, velocity_unit);
    
    // Create maneuver along velocity direction
    out_maneuver->delta_v[0] = velocity_unit[0] * required_dv_ms;
    out_maneuver->delta_v[1] = velocity_unit[1] * required_dv_ms;
    out_maneuver->delta_v[2] = velocity_unit[2] * required_dv_ms;
    
    // Set execution time to encounter time (or slightly before for practical execution)
    out_maneuver->epoch = encounter_time;
    
    // Generate deterministic ID
    long long epoch_int = (long long)(encounter_time * 1000000); // Microsecond precision
    snprintf(out_maneuver->id, sizeof(out_maneuver->id), "AVOID_%lld", epoch_int);
    
    // Estimate fuel cost using default parameters
    // Note: Using sentinel values for masses as they may not be available
    // Caller should validate fuel availability separately
    double dry_mass = 1000.0;      // kg, placeholder
    double prop_mass = 100.0;      // kg, placeholder
    double efficiency = 1.0;       // Perfect efficiency
    
    out_maneuver->fuel_cost = fuel_consumption(required_dv_ms / 1000.0, // Convert to km/s
                                              isp_default_chemical(),
                                              dry_mass, prop_mass, efficiency);
    
    // If masses are not meaningful, set fuel_cost to -1.0 as sentinel
    // TODO: Source actual spacecraft masses from repository if available
    if (dry_mass <= 0.0 || prop_mass <= 0.0) {
        out_maneuver->fuel_cost = -1.0;
    }
    
    return 0; // Success
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
