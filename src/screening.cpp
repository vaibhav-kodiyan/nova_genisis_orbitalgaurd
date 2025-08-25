#include "simplified_core.h"
#include "types.h" // for severity_to_string

vector<Encounter> screen_by_threshold(
    const vector<Trajectory>& tracks, 
    double threshold_m) {
    
    vector<Encounter> encounters;
    
    if (tracks.size() < 2) {
        return encounters;
    }
    
    // Find minimum number of time steps across all trajectories
    size_t minSteps = tracks[0].states.size();
    for (const auto& track : tracks) {
        if (track.states.size() < minSteps) {
            minSteps = track.states.size();
        }
    }
    
    // Pairwise distance check at each time step
    for (size_t i = 0; i < tracks.size() - 1; ++i) {
        for (size_t j = i + 1; j < tracks.size(); ++j) {
            const auto& track1 = tracks[i];
            const auto& track2 = tracks[j];
            
            // Check each time-aligned sample
            for (size_t k = 0; k < minSteps; ++k) {
                const auto& state1 = track1.states[k];
                const auto& state2 = track2.states[k];
                
                // Compute Euclidean distance in meters
                double dx = (state1.x - state2.x) * 1000.0; // Convert km to m
                double dy = (state1.y - state2.y) * 1000.0;
                double dz = (state1.z - state2.z) * 1000.0;
                double distance_m = sqrt(dx*dx + dy*dy + dz*dz);
                
                // Check threshold (caller-provided threshold may already account for object radii)
                if (distance_m <= threshold_m) {
                    // Severity bands relative to threshold
                    // <= 1/3 threshold: High, <= 2/3: Medium, <= threshold: Low, else: None
                    int level = NONE;
                    if (distance_m <= (threshold_m / 3.0)) {
                        level = HIGH;
                    } else if (distance_m <= (2.0 * threshold_m / 3.0)) {
                        level = MEDIUM;
                    } else { // <= threshold_m
                        level = LOW;
                    }

                    // Map to string (as requested). Not stored; kept simple.
                    (void)severity_to_string(level);

                    Encounter encounter;
                    encounter.aId = track1.id;
                    encounter.bId = track2.id;
                    encounter.t = state1.t;
                    encounter.miss_m = distance_m;
                    encounter.rel_mps = 0.0; // keep simple: no velocity-based logic
                    
                    encounters.push_back(encounter);
                    
                    // Record only first hit per pair to avoid duplicates
                    break;
                }
            }
        }
    }
    
    return encounters;
}
