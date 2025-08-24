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

extern "C" {

// Error reporting
const char* og_last_error(void) {
    return g_last_error[0] ? g_last_error : nullptr;
}

// Handles & lifecycle - stubs
void* og_parse_tle(const char* name, const char* line1, const char* line2) {
    clear_error();
    
    if (!validate_ptr(name, "name") || 
        !validate_ptr(line1, "line1") || 
        !validate_ptr(line2, "line2")) {
        return nullptr;
    }
    
    // TODO: Implement TLE parsing
    set_error("og_parse_tle not yet implemented");
    return nullptr;
}

void og_free_elements(void* elements) {
    clear_error();
    
    if (!elements) {
        return; // Silently ignore null pointer (like free())
    }
    
    // TODO: Implement element cleanup
    set_error("og_free_elements not yet implemented");
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
