#include "simplified_core.h"
#include "types.h"
 

vector<Trajectory> propagate_coords_only(
    const vector<string>& ids,
    const vector<bool>& isDebrisFlags,
    double startEpochMs,
    double stepSeconds,
    double durationHours) {
    
    vector<Trajectory> trajectories;
    
    // Fallback: create dummy trajectories for testing
    (void)ids; (void)isDebrisFlags; // Suppress unused parameter warnings
    
    Trajectory dummyTraj;
    dummyTraj.id = "TEST_SAT";
    dummyTraj.isDebris = false;
    
    const double totalMinutes = durationHours * 60.0;
    const double stepMinutes = stepSeconds / 60.0;
    const int numSteps = static_cast<int>(totalMinutes / stepMinutes) + 1;
    
    for (int step = 0; step < numSteps; ++step) {
        State state;
        state.t = startEpochMs + step * stepMinutes * 60000.0;
        state.x = 7000.0; // Dummy orbit at ~7000km
        state.y = 0.0;
        state.z = 0.0;
        state.vx = 0.0;
        state.vy = 7.5;
        state.vz = 0.0;
        state.rad = 7000.0;
        
        dummyTraj.states.push_back(state);
    }
    
    trajectories.push_back(dummyTraj);
    
    return trajectories;
}