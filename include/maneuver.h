#ifndef MANEUVER_H
#define MANEUVER_H

#include "types.h"

// Maneuver execution structure
struct Maneuver {
    char   id[32];        // deterministic, human-readable (no RNG)
    double epoch;         // execution time (same unit as repo timebase - Julian date)
    double delta_v[3];    // ECI components, m/s
    double fuel_cost;     // kg
};

/**
 * Calculate fuel consumption for a given delta-v maneuver using rocket equation
 * 
 * @param delta_v_km_s Delta-v magnitude in km/s
 * @param specific_impulse_s Specific impulse in seconds
 * @param dry_mass_kg Dry mass of spacecraft in kg
 * @param propellant_mass_kg Available propellant mass in kg
 * @param efficiency Propulsion efficiency (0-1 scalar)
 * @return Required fuel mass in kg
 */
double fuel_consumption(double delta_v_km_s, double specific_impulse_s, 
                       double dry_mass_kg, double propellant_mass_kg, double efficiency);

/**
 * Plan an avoidance maneuver between two objects
 * 
 * @param primary Primary object orbital elements
 * @param secondary Secondary object orbital elements  
 * @param encounter_time Time of closest approach (Julian date)
 * @param target_distance_km Desired separation distance in km
 * @param max_delta_v_mps Maximum allowable delta-v in m/s
 * @param out_maneuver Output maneuver structure
 * @return 0 on success, non-zero on failure
 */
int plan_avoidance(const OrbitalElements* primary, const OrbitalElements* secondary,
                  double encounter_time, double target_distance_km, 
                  double max_delta_v_mps, Maneuver* out_maneuver);

/**
 * Apply a maneuver to orbital elements and compute resulting state vector
 * 
 * @param elements Initial orbital elements
 * @param m Maneuver to apply
 * @param current_unix_ms Current time in milliseconds since Unix epoch
 * @param out_state Output state vector in ECI coordinates
 */
void apply_maneuver(const OrbitalElements* elements, const Maneuver* m,
                   double current_unix_ms, StateVectorECI* out_state);

/**
 * Get default chemical propulsion specific impulse
 * @return Default Isp in seconds (300 s)
 */
double isp_default_chemical();

/**
 * Get electric propulsion specific impulse (for future use)
 * @return Electric Isp in seconds (3000 s)
 */
double isp_electric();

#endif // MANEUVER_H
