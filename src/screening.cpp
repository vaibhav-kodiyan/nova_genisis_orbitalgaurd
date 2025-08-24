#include "screening.h"
#include <cmath>
#include <cstring>
#include <algorithm>

double distance3d(const double* pos1, const double* pos2) {
    if (!pos1 || !pos2) {
        return -1.0; // Invalid input
    }
    
    double dx = pos1[0] - pos2[0];
    double dy = pos1[1] - pos2[1];
    double dz = pos1[2] - pos2[2];
    
    return sqrt(dx*dx + dy*dy + dz*dz);
}

int classify_severity(double distance) {
    if (distance < 0) {
        return NONE;
    }
    
    if (distance <= SEVERITY_THRESHOLD_1KM) {
        return CRASH;
    } else if (distance <= SEVERITY_THRESHOLD_5KM) {
        return HIGH;
    } else if (distance <= SEVERITY_THRESHOLD_25KM) {
        return MEDIUM;
    } else {
        return LOW;
    }
}

double logistic_probability(double distance, double relative_velocity) {
    if (distance < 0 || relative_velocity < 0) {
        return 0.0;
    }
    
    // Enhanced logistic function that considers both distance and relative velocity
    // Higher relative velocity increases collision probability
    double velocity_factor = 1.0 + (relative_velocity / 10.0); // Normalize velocity impact
    double adjusted_distance = distance / velocity_factor;
    
    // Standard logistic function: 1 / (1 + exp(k * (x - x0)))
    double exponent = LOGISTIC_K * (adjusted_distance - LOGISTIC_X0_KM);
    return 1.0 / (1.0 + exp(exponent));
}

double compute_relative_velocity(const StateVectorECI* state1, const StateVectorECI* state2) {
    if (!state1 || !state2) {
        return 0.0;
    }
    
    double dvx = state1->v[0] - state2->v[0];
    double dvy = state1->v[1] - state2->v[1];
    double dvz = state1->v[2] - state2->v[2];
    
    return sqrt(dvx*dvx + dvy*dvy + dvz*dvz);
}

int screen_conjunctions(const StateVectorECI** trajectories, const char** ids, 
                       size_t sat_count, double max_distance_km, 
                       Encounter* out_encounters, size_t* out_count, size_t max_out) {
    
    // Input validation
    if (!trajectories || !ids || !out_encounters || !out_count || sat_count < 2) {
        return SCREENING_ERROR_INVALID_INPUT;
    }
    
    *out_count = 0;
    size_t encounter_index = 0;
    
    // Pairwise comparison of all satellites
    for (size_t i = 0; i < sat_count - 1; i++) {
        for (size_t j = i + 1; j < sat_count; j++) {
            if (!trajectories[i] || !trajectories[j] || !ids[i] || !ids[j]) {
                continue; // Skip invalid entries
            }
            
            // Find minimum distance between satellite pair
            double min_distance = max_distance_km + 1.0; // Initialize above threshold
            double time_of_closest = 0.0;
            double rel_velocity_at_closest = 0.0;
            int closest_index = -1;
            
            // Assume trajectories have synchronized time steps
            // For simplicity, assume 100 time steps (this should be configurable)
            const int MAX_TIME_STEPS = 100;
            
            for (int t = 0; t < MAX_TIME_STEPS; t++) {
                const StateVectorECI* state1 = &trajectories[i][t];
                const StateVectorECI* state2 = &trajectories[j][t];
                
                // Check if states are time-synchronized within tolerance
                double time_diff = fabs(state1->t - state2->t);
                if (time_diff > SYNC_TOLERANCE_S / SECONDS_PER_DAY) {
                    continue; // Skip unsynchronized states
                }
                
                double dist = distance3d(state1->r, state2->r);
                
                if (dist < min_distance) {
                    min_distance = dist;
                    time_of_closest = state1->t;
                    rel_velocity_at_closest = compute_relative_velocity(state1, state2);
                    closest_index = t;
                }
            }
            
            // Check if encounter meets distance criteria
            if (min_distance <= max_distance_km && closest_index >= 0) {
                if (encounter_index >= max_out) {
                    return SCREENING_ERROR_INSUFFICIENT_BUFFER;
                }
                
                // Create encounter record
                Encounter* enc = &out_encounters[encounter_index];
                
                // Copy satellite IDs (safely)
                strncpy(enc->sat1_id, ids[i], sizeof(enc->sat1_id) - 1);
                enc->sat1_id[sizeof(enc->sat1_id) - 1] = '\0';
                strncpy(enc->sat2_id, ids[j], sizeof(enc->sat2_id) - 1);
                enc->sat2_id[sizeof(enc->sat2_id) - 1] = '\0';
                
                enc->time_of_closest_approach = time_of_closest;
                enc->min_distance_km = min_distance;
                enc->relative_velocity = rel_velocity_at_closest;
                enc->severity = classify_severity(min_distance);
                enc->collision_probability = logistic_probability(min_distance, rel_velocity_at_closest);
                
                encounter_index++;
            }
        }
    }
    
    *out_count = encounter_index;
    return SCREENING_SUCCESS;
}

// Comparison function for sorting by risk (probability * severity factor)
static int compare_by_risk(const void* a, const void* b) {
    const Encounter* enc_a = (const Encounter*)a;
    const Encounter* enc_b = (const Encounter*)b;
    
    double risk_a = enc_a->collision_probability * (enc_a->severity + 1);
    double risk_b = enc_b->collision_probability * (enc_b->severity + 1);
    
    if (risk_a > risk_b) return -1;
    if (risk_a < risk_b) return 1;
    return 0;
}

// Comparison function for sorting by time
static int compare_by_time(const void* a, const void* b) {
    const Encounter* enc_a = (const Encounter*)a;
    const Encounter* enc_b = (const Encounter*)b;
    
    if (enc_a->time_of_closest_approach < enc_b->time_of_closest_approach) return -1;
    if (enc_a->time_of_closest_approach > enc_b->time_of_closest_approach) return 1;
    return 0;
}

void sort_encounters_by_risk(Encounter* encounters, size_t count) {
    if (!encounters || count == 0) {
        return;
    }
    
    qsort(encounters, count, sizeof(Encounter), compare_by_risk);
}

void sort_encounters_by_time(Encounter* encounters, size_t count) {
    if (!encounters || count == 0) {
        return;
    }
    
    qsort(encounters, count, sizeof(Encounter), compare_by_time);
}

size_t filter_by_probability(Encounter* encounters, size_t count, double min_probability) {
    if (!encounters || count == 0 || min_probability < 0.0) {
        return count;
    }
    
    size_t write_index = 0;
    
    for (size_t read_index = 0; read_index < count; read_index++) {
        if (encounters[read_index].collision_probability >= min_probability) {
            if (write_index != read_index) {
                encounters[write_index] = encounters[read_index];
            }
            write_index++;
        }
    }
    
    return write_index;
}
