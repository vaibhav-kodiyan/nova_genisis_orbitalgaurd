#include "api.h"
#include "types.h"
#include "propagation.h"
#include "screening.h"
#include "constants.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <string>
#include <sstream>

// Thread-local error storage
static thread_local char g_last_error[256] = {0};

// Helper function to set error message
static void set_error(const char* message) {
    if (message) {
        strncpy(g_last_error, message, sizeof(g_last_error) - 1);
        g_last_error[sizeof(g_last_error) - 1] = '\0';
    } else {
        g_last_error[0] = '\0';
    }
}

// Helper function to clear error
static void clear_error() {
    g_last_error[0] = '\0';
}

// Helper function to validate null pointers
static bool validate_ptr(const void* ptr, const char* param_name) {
    if (!ptr) {
        std::string error_msg = "Invalid parameter: ";
        error_msg += param_name;
        error_msg += " is null";
        set_error(error_msg.c_str());
        return false;
    }
    return true;
}

// Helper function to parse TLE lines into orbital elements
static bool parse_tle_to_elements(const char* name, const char* line1, const char* line2, OrbitalElements* elements) {
    if (!line1 || !line2 || !elements || strlen(line1) < 69 || strlen(line2) < 69) {
        return false;
    }
    
    // Validate TLE format
    if (line1[0] != '1' || line2[0] != '2') {
        return false;
    }
    
    try {
        // Parse Line 1
        elements->epoch = 0.0; // Will be computed from epoch year and day
        elements->bstar = 0.0;
        elements->ndot = 0.0;
        elements->nddot = 0.0;
        
        // Extract epoch year and day from line 1 (columns 19-32)
        std::string epoch_str(line1 + 18, 14);
        double epoch_year = std::stod(epoch_str.substr(0, 2));
        double epoch_day = std::stod(epoch_str.substr(2));
        
        // Convert 2-digit year to 4-digit (assume 1957-2056 range)
        if (epoch_year < 57) {
            epoch_year += 2000;
        } else {
            epoch_year += 1900;
        }
        
        // Convert to Julian date (approximate)
        elements->epoch = 365.25 * (epoch_year - 2000) + JULIAN_EPOCH + epoch_day - 1;
        
        // Parse Line 2
        std::string line2_str(line2);
        
        // Inclination (columns 9-16)
        elements->inclination = std::stod(line2_str.substr(8, 8)) * DEG_TO_RAD;
        elements->tilt = elements->inclination; // Alias
        
        // RAAN (columns 18-25)
        elements->raan = std::stod(line2_str.substr(17, 8)) * DEG_TO_RAD;
        elements->node = elements->raan; // Alias
        
        // Eccentricity (columns 27-33, implied decimal point)
        std::string ecc_str = "0." + line2_str.substr(26, 7);
        elements->eccentricity = std::stod(ecc_str);
        
        // Argument of perigee (columns 35-42)
        elements->arg_perigee = std::stod(line2_str.substr(34, 8)) * DEG_TO_RAD;
        elements->perigee_angle = elements->arg_perigee; // Alias
        
        // Mean anomaly (columns 44-51)
        elements->mean_anomaly = std::stod(line2_str.substr(43, 8)) * DEG_TO_RAD;
        elements->position = elements->mean_anomaly; // Alias
        
        // Mean motion (columns 53-63)
        elements->mean_motion = std::stod(line2_str.substr(52, 11));
        
        // Calculate semi-major axis from mean motion
        double n_rad_per_sec = elements->mean_motion * TWO_PI / (24.0 * 3600.0);
        elements->semi_major_axis = pow(MU / (n_rad_per_sec * n_rad_per_sec), 1.0/3.0);
        
        // Set time alias
        elements->time = elements->epoch;
        
        return true;
    } catch (...) {
        return false;
    }
}

extern "C" {

// Error reporting
const char* og_last_error(void) {
    return g_last_error[0] ? g_last_error : nullptr;
}

// Handles & lifecycle
void* og_parse_tle(const char* name, const char* line1, const char* line2) {
    clear_error();
    
    if (!validate_ptr(name, "name") || 
        !validate_ptr(line1, "line1") || 
        !validate_ptr(line2, "line2")) {
        return nullptr;
    }
    
    // Allocate orbital elements structure
    OrbitalElements* elements = new(std::nothrow) OrbitalElements;
    if (!elements) {
        set_error("Memory allocation failed");
        return nullptr;
    }
    
    // Initialize to zero
    memset(elements, 0, sizeof(OrbitalElements));
    
    // Parse TLE data
    if (!parse_tle_to_elements(name, line1, line2, elements)) {
        delete elements;
        set_error("Failed to parse TLE data");
        return nullptr;
    }
    
    return elements;
}

void og_free_elements(void* elements) {
    clear_error();
    
    if (!elements) {
        return; // Silently ignore null pointer (like free())
    }
    
    delete static_cast<OrbitalElements*>(elements);
}

// Propagation
int og_propagate(void* elements, double minutes, 
                double* out_state3, double* out_vel3) {
    clear_error();
    
    if (!validate_ptr(elements, "elements") ||
        !validate_ptr(out_state3, "out_state3") ||
        !validate_ptr(out_vel3, "out_vel3")) {
        return -1;
    }
    
    OrbitalElements* elem = static_cast<OrbitalElements*>(elements);
    StateVectorECI state;
    
    // Call internal propagation function
    int result = propagate(elem, minutes, &state);
    
    if (result == PROPAGATION_SUCCESS) {
        // Copy position and velocity to output arrays
        out_state3[0] = state.r[0];
        out_state3[1] = state.r[1];
        out_state3[2] = state.r[2];
        
        out_vel3[0] = state.v[0];
        out_vel3[1] = state.v[1];
        out_vel3[2] = state.v[2];
        
        return 0;
    } else {
        // Map internal error codes to API error codes
        switch (result) {
            case PROPAGATION_ERROR_INVALID_INPUT:
                set_error("Invalid input parameters for propagation");
                break;
            case PROPAGATION_ERROR_CONVERGENCE:
                set_error("Kepler equation failed to converge");
                break;
            case PROPAGATION_ERROR_NAN_RESULT:
                set_error("Propagation resulted in NaN values");
                break;
            default:
                set_error("Unknown propagation error");
                break;
        }
        return result;
    }
}

// Screening
size_t og_screen(const double* const* states, const char** ids,
                size_t sat_count, double max_distance_km,
                og_encounter_t* out_encounters, size_t max_encounters) {
    clear_error();
    
    if (!validate_ptr(states, "states") ||
        !validate_ptr(ids, "ids") ||
        !validate_ptr(out_encounters, "out_encounters")) {
        return 0;
    }
    
    if (sat_count < 2) {
        set_error("Need at least 2 satellites for screening");
        return 0;
    }
    
    if (max_encounters == 0) {
        return 0;
    }
    
    size_t encounter_count = 0;
    
    // Simple pairwise distance screening (single time point)
    for (size_t i = 0; i < sat_count - 1 && encounter_count < max_encounters; i++) {
        for (size_t j = i + 1; j < sat_count && encounter_count < max_encounters; j++) {
            if (!states[i] || !states[j] || !ids[i] || !ids[j]) {
                continue; // Skip invalid entries
            }
            
            // Calculate distance between satellites
            double dx = states[i][0] - states[j][0];
            double dy = states[i][1] - states[j][1];
            double dz = states[i][2] - states[j][2];
            double distance = sqrt(dx*dx + dy*dy + dz*dz);
            
            // Check if encounter meets distance criteria
            if (distance <= max_distance_km) {
                og_encounter_t* enc = &out_encounters[encounter_count];
                
                // Copy satellite IDs (safely)
                strncpy(enc->id_a, ids[i], sizeof(enc->id_a) - 1);
                enc->id_a[sizeof(enc->id_a) - 1] = '\0';
                strncpy(enc->id_b, ids[j], sizeof(enc->id_b) - 1);
                enc->id_b[sizeof(enc->id_b) - 1] = '\0';
                
                enc->min_distance_km = distance;
                enc->tca_epoch = 0.0; // Current time (not specified in input)
                
                encounter_count++;
            }
        }
    }
    
    return encounter_count;
}

// Maneuver planning
int og_plan_maneuver(const void* primary_elements, const void* secondary_elements,
                    double encounter_epoch, double target_distance_km,
                    double max_delta_v_mps, og_maneuver_t* out_m) {
    clear_error();
    
    if (!validate_ptr(primary_elements, "primary_elements") ||
        !validate_ptr(secondary_elements, "secondary_elements") ||
        !validate_ptr(out_m, "out_m")) {
        return -1;
    }
    
    if (target_distance_km <= 0 || max_delta_v_mps <= 0) {
        set_error("Invalid maneuver parameters");
        return -1;
    }
    
    // Basic maneuver planning (simplified approach)
    // In a real implementation, this would use Lambert's problem or similar
    OrbitalElements* primary = static_cast<OrbitalElements*>(const_cast<void*>(primary_elements));
    OrbitalElements* secondary = static_cast<OrbitalElements*>(const_cast<void*>(secondary_elements));
    
    // Calculate time to encounter from primary's epoch
    double time_to_encounter = (encounter_epoch - primary->epoch) * MINUTES_PER_DAY;
    
    // Propagate both satellites to encounter time
    StateVectorECI primary_state, secondary_state;
    int result1 = propagate(primary, time_to_encounter, &primary_state);
    int result2 = propagate(secondary, time_to_encounter, &secondary_state);
    
    if (result1 != PROPAGATION_SUCCESS || result2 != PROPAGATION_SUCCESS) {
        set_error("Failed to propagate satellites to encounter time");
        return -1;
    }
    
    // Calculate relative position
    double rel_pos[3] = {
        secondary_state.r[0] - primary_state.r[0],
        secondary_state.r[1] - primary_state.r[1],
        secondary_state.r[2] - primary_state.r[2]
    };
    
    double current_distance = sqrt(rel_pos[0]*rel_pos[0] + rel_pos[1]*rel_pos[1] + rel_pos[2]*rel_pos[2]);
    
    if (current_distance > target_distance_km) {
        // No maneuver needed
        memset(out_m, 0, sizeof(og_maneuver_t));
        out_m->epoch = encounter_epoch;
        out_m->fuel_cost_kg = 0.0;
        strncpy(out_m->id, "PRIMARY", sizeof(out_m->id) - 1);
        return 0;
    }
    
    // Simple avoidance maneuver: radial delta-V
    double separation_needed = target_distance_km - current_distance;
    double delta_v_magnitude = separation_needed * 0.001; // Simplified conversion (km to km/s)
    
    // Convert to m/s and check against limit
    double delta_v_ms = delta_v_magnitude * 1000.0;
    if (delta_v_ms > max_delta_v_mps) {
        set_error("Required delta-V exceeds maximum allowed");
        return -1;
    }
    
    // Fill output structure
    out_m->epoch = encounter_epoch;
    out_m->delta_v[0] = delta_v_ms; // Radial component
    out_m->delta_v[1] = 0.0;
    out_m->delta_v[2] = 0.0;
    out_m->fuel_cost_kg = -1.0; // Unknown without spacecraft mass
    strncpy(out_m->id, "PRIMARY", sizeof(out_m->id) - 1);
    out_m->id[sizeof(out_m->id) - 1] = '\0';
    
    return 0;
}

// Fuel consumption - stub with basic rocket equation
double og_fuel_consumption(double delta_v_km_s, double specific_impulse_s,
                          double dry_mass_kg, double propellant_mass_kg, 
                          double efficiency) {
    clear_error();
    
    // Input validation
    if (delta_v_km_s < 0 || specific_impulse_s <= 0 || 
        dry_mass_kg <= 0 || propellant_mass_kg < 0 || 
        efficiency <= 0 || efficiency > 1.0) {
        set_error("Invalid fuel consumption parameters");
        return -1.0;
    }
    
    // Convert delta-v to m/s
    double delta_v_ms = delta_v_km_s * 1000.0;
    
    // Standard gravity (m/s²)
    const double g0 = 9.80665;
    
    // Effective specific impulse accounting for efficiency
    double eff_isp = specific_impulse_s * efficiency;
    
    // Rocket equation: m_fuel = m_total * (1 - exp(-dv/(g0*Isp)))
    double total_mass = dry_mass_kg + propellant_mass_kg;
    double mass_ratio = exp(delta_v_ms / (g0 * eff_isp));
    double required_fuel = total_mass * (mass_ratio - 1.0);
    
    // Check if we have enough propellant
    if (required_fuel > propellant_mass_kg) {
        return -1.0; // Insufficient propellant
    }
    
    return required_fuel;
}

} // extern "C"
