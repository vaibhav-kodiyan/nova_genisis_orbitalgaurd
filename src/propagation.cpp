#include "simplified_core.h"
#include "types.h"
#ifdef ENABLE_SGP4
#include "SGP4/SGP4/SGP4.h"
#endif

std::vector<Trajectory> propagate_coords_only(
    const std::vector<std::string>& ids,
    const std::vector<bool>& isDebrisFlags,
    double startEpochMs,
    double stepSeconds,
    double durationHours) {
    
    std::vector<Trajectory> trajectories;
    
#ifdef ENABLE_SGP4
    // Load TLEs from data file
    std::vector<TLE> tles = parseTLEfile("data/sample3.tle");
    if (tles.empty()) {
        return trajectories;
    }
    
    trajectories.reserve(tles.size());
    
    // Build SGP4 satellite records
    std::vector<elsetrec> satrecs;
    std::vector<std::string> names;
    satrecs.reserve(tles.size());
    names.reserve(tles.size());
    
    for (auto &t : tles) {
        elsetrec rec{};
        double startmfe = 0.0, stopmfe = 0.0, deltamin = 0.0;
        char l1[130], l2[130];
        
        // Copy TLE lines to mutable C strings
        size_t i = 0;
        while (t.line1[i] != '\0' && i + 1 < sizeof(l1)) {
            l1[i] = t.line1[i];
            ++i;
        }
        l1[i] = '\0';
        
        i = 0;
        while (t.line2[i] != '\0' && i + 1 < sizeof(l2)) {
            l2[i] = t.line2[i];
            ++i;
        }
        l2[i] = '\0';
        
        // Initialize SGP4 record
        SGP4Funcs::twoline2rv(l1, l2, 'v', 'e', 'i', wgs84, startmfe, stopmfe, deltamin, rec);
        satrecs.push_back(rec);
        names.push_back(t.name);
    }
    
    // Generate time-stepped coordinates for each satellite
    const double totalMinutes = durationHours * 60.0;
    const double stepMinutes = stepSeconds / 60.0;
    const int numSteps = static_cast<int>(totalMinutes / stepMinutes) + 1;
    
    for (size_t satIdx = 0; satIdx < satrecs.size(); ++satIdx) {
        Trajectory traj;
        traj.id = names[satIdx];
        traj.isDebris = (satIdx > 0); // First satellite is primary, rest are debris
        traj.states.reserve(numSteps);
        
        for (int step = 0; step < numSteps; ++step) {
            double currentTimeMinutes = step * stepMinutes;
            double r[3] = {0, 0, 0};
            double v[3] = {0, 0, 0};
            
            // Use SGP4 to propagate satellite position
            SGP4Funcs::sgp4(satrecs[satIdx], currentTimeMinutes, r, v);
            
            State state;
            state.t = startEpochMs + currentTimeMinutes * 60000.0; // Convert to milliseconds
            state.x = r[0];
            state.y = r[1];
            state.z = r[2];
            state.vx = v[0];
            state.vy = v[1];
            state.vz = v[2];
            state.rad = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
            
            traj.states.push_back(state);
        }
        
        trajectories.push_back(traj);
    }
#else
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
#endif
    
    return trajectories;
}
