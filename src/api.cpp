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

// Propagation - stub
int og_propagate(void* elements, double minutes, 
                double* out_state3, double* out_vel3) {
    clear_error();
    
    if (!validate_ptr(elements, "elements") ||
        !validate_ptr(out_state3, "out_state3") ||
        !validate_ptr(out_vel3, "out_vel3")) {
        return -1;
    }
    
    // TODO: Implement propagation bridge
    set_error("og_propagate not yet implemented");
    return -1;
}

// Screening - stub
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
    
    // TODO: Implement screening bridge
    set_error("og_screen not yet implemented");
    return 0;
}

// Maneuver planning - stub
int og_plan_maneuver(const void* primary_elements, const void* secondary_elements,
                    double encounter_epoch, double target_distance_km,
                    double max_delta_v_mps, og_maneuver_t* out_m) {
    clear_error();
    
    if (!validate_ptr(primary_elements, "primary_elements") ||
        !validate_ptr(secondary_elements, "secondary_elements") ||
        !validate_ptr(out_m, "out_m")) {
        return -1;
    }
    
    // TODO: Implement maneuver planning
    set_error("og_plan_maneuver not yet implemented");
    return -1;
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
