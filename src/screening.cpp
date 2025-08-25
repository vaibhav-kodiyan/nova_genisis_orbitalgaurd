#include "simplified_core.h"

std::vector<Encounter> screen_by_threshold(
    const std::vector<Trajectory>& tracks, 
    double threshold_m) {
    
    std::vector<Encounter> encounters;
    
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
                
                // Check threshold
                if (distance_m <= threshold_m) {
                    // Compute relative speed if velocities are available
                    double dvx = (state1.vx - state2.vx) * 1000.0; // Convert km/s to m/s
                    double dvy = (state1.vy - state2.vy) * 1000.0;
                    double dvz = (state1.vz - state2.vz) * 1000.0;
                    double rel_speed_mps = sqrt(dvx*dvx + dvy*dvy + dvz*dvz);
                    
                    Encounter encounter;
                    encounter.aId = track1.id;
                    encounter.bId = track2.id;
                    encounter.t = state1.t;
                    encounter.miss_m = distance_m;
                    encounter.rel_mps = rel_speed_mps;
                    
                    encounters.push_back(encounter);
                    
                    // Record only first hit per pair to avoid duplicates
                    break;
                }
            }
        }
    }
    
    return encounters;
}
