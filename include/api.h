#ifndef OG_API_H
#define OG_API_H

#include <stddef.h>
#include "time_utils.h"

// Symbol visibility macro
#if defined(_WIN32)
#  define OG_API __declspec(dllexport)
#else
#  define OG_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * C ABI structures safe for C
 */

typedef struct {
    gregorian_time_t tca_time; // Time of closest approach (Gregorian calendar)
    double min_distance_km;    // Minimum distance in kilometers
    char   id_a[32];          // First satellite ID
    char   id_b[32];          // Second satellite ID
} og_encounter_t;

typedef struct {
    gregorian_time_t time;     // Maneuver time (Gregorian calendar)
    double delta_v[3];         // Delta-V vector in m/s, ECI coordinates
    double fuel_cost_kg;       // Fuel cost in kg (-1 if unknown)
    char   id[32];            // Satellite ID
} og_maneuver_t;

/**
 * Handles & lifecycle
 */

/**
 * Parse TLE data and return orbital elements handle
 * @param name Satellite name
 * @param line1 First line of TLE
 * @param line2 Second line of TLE
 * @return Opaque handle to orbital elements (NULL on error)
 */
OG_API void* og_parse_tle(const char* name, const char* line1, const char* line2);

/**
 * Free orbital elements handle
 * @param elements Handle returned by og_parse_tle
 */
OG_API void og_free_elements(void* elements);

/**
 * Propagation (minutes input as per spec)
 */

/**
 * Propagate orbital elements to specified time
 * @param elements Handle to orbital elements
 * @param minutes Time offset from epoch in minutes
 * @param out_state3 Output position vector [x,y,z] in km
 * @param out_vel3 Output velocity vector [vx,vy,vz] in km/s
 * @return 0 on success, non-zero on error
 */
OG_API int og_propagate(void* elements, double minutes, 
                       double* out_state3, double* out_vel3);

/**
 * Screening (flat arrays, no C++ types)
 */

/**
 * Screen for conjunctions between satellites
 * @param states Array of pointers to position vectors [x,y,z] in km
 * @param ids Array of null-terminated satellite IDs
 * @param sat_count Number of satellites
 * @param max_distance_km Maximum distance threshold for encounters
 * @param out_encounters Output array for detected encounters
 * @param max_encounters Maximum number of encounters that can be stored
 * @return Number of encounters found, or negative on error
 */
OG_API size_t og_screen(const double* const* states, const char** ids,
                       size_t sat_count, double max_distance_km,
                       og_encounter_t* out_encounters, size_t max_encounters);

/**
 * Maneuver planning & fuel helpers
 */

/**
 * Plan maneuver to avoid collision
 * @param primary_elements Handle to primary satellite elements
 * @param secondary_elements Handle to secondary satellite elements
 * @param encounter_time Time of encounter (Gregorian calendar)
 * @param target_distance_km Desired separation distance
 * @param max_delta_v_mps Maximum allowed delta-V in m/s
 * @param out_m Output maneuver structure
 * @return 0 on success, non-zero on error
 */
OG_API int og_plan_maneuver(const void* primary_elements, const void* secondary_elements,
                           const gregorian_time_t* encounter_time, double target_distance_km,
                           double max_delta_v_mps, og_maneuver_t* out_m);

/**
 * Calculate fuel consumption for a given delta-V
 * @param delta_v_km_s Delta-V magnitude in km/s
 * @param specific_impulse_s Specific impulse in seconds
 * @param dry_mass_kg Dry mass in kg
 * @param propellant_mass_kg Available propellant mass in kg
 * @param efficiency Propulsion system efficiency (0.0-1.0)
 * @return Fuel consumption in kg, or negative on error
 */
OG_API double og_fuel_consumption(double delta_v_km_s, double specific_impulse_s,
                                 double dry_mass_kg, double propellant_mass_kg, 
                                 double efficiency);

/**
 * Error reporting
 */

/**
 * Get last error message
 * @return Pointer to error string, or NULL if no error
 */
OG_API const char* og_last_error(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // OG_API_H
