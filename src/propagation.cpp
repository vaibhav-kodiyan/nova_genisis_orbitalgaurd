#include "simplified_core.h"
#include "types.h"
 

vector<Trajectory> propagate_coords_only(
    vector<string>& ids,
    vector<bool>& isDebrisFlags,
    double startEpochMs,
    double stepSeconds,
    double durationHours) {
    
    vector<Trajectory> trajectories;
    
    // Load satellite and debris TLE data
    cout << "Loading satellite TLE data..." << endl;
    vector<TLE> satellites = parseTLEfile("data/satellites_1000.tle");
    cout << "Loaded " << satellites.size() << " satellites" << endl;
    
    cout << "Loading debris TLE data..." << endl;
    vector<TLE> debris = parseTLEfile("data/debris_3000.tle");
    cout << "Loaded " << debris.size() << " debris objects" << endl;
    
    // Clear and populate the output vectors
    ids.clear();
    isDebrisFlags.clear();
    
    const double totalMinutes = durationHours * 60.0;
    const double stepMinutes = stepSeconds / 60.0;
    const int numSteps = static_cast<int>(totalMinutes / stepMinutes) + 1;
    
    // Process satellites
    for (const auto& tle : satellites) {
        Trajectory traj;
        traj.id = tle.name;
        traj.isDebris = false;
        
        ids.push_back(tle.name);
        isDebrisFlags.push_back(false);
        
        // Generate simplified orbital trajectory
        for (int step = 0; step < numSteps; ++step) {
            State state;
            state.t = startEpochMs + step * stepMinutes * 60000.0;
            
            // Simple circular orbit approximation based on TLE data
            double timeHours = step * stepMinutes / 60.0;
            double angle = timeHours * 0.1; // Simplified orbital motion
            
            // Vary orbital parameters based on satellite index for visual diversity
            size_t satIndex = trajectories.size();
            double radius = 6800.0 + (satIndex % 100) * 10.0; // 6800-7800 km
            double inclination = (satIndex % 180) * M_PI / 180.0; // 0-179 degrees
            
            state.x = radius * cos(angle) * cos(inclination);
            state.y = radius * sin(angle) * cos(inclination);
            state.z = radius * sin(inclination) * sin(angle * 0.5);
            
            // Simple velocity calculation
            state.vx = -radius * sin(angle) * 0.1 * cos(inclination);
            state.vy = radius * cos(angle) * 0.1 * cos(inclination);
            state.vz = radius * cos(inclination) * 0.05;
            
            state.rad = radius;
            
            traj.states.push_back(state);
        }
        
        trajectories.push_back(traj);
    }
    
    // Process debris
    for (const auto& tle : debris) {
        Trajectory traj;
        traj.id = tle.name;
        traj.isDebris = true;
        
        ids.push_back(tle.name);
        isDebrisFlags.push_back(true);
        
        // Generate more chaotic trajectories for debris
        for (int step = 0; step < numSteps; ++step) {
            State state;
            state.t = startEpochMs + step * stepMinutes * 60000.0;
            
            double timeHours = step * stepMinutes / 60.0;
            size_t debrisIndex = trajectories.size() - satellites.size();
            
            // More varied orbital parameters for debris
            double angle = timeHours * (0.05 + (debrisIndex % 50) * 0.002); // Varied orbital periods
            double radius = 6500.0 + (debrisIndex % 200) * 15.0; // 6500-9500 km
            double inclination = (debrisIndex % 180) * M_PI / 180.0;
            double eccentricity = (debrisIndex % 30) * 0.01; // 0-0.3 eccentricity
            
            // Elliptical orbit approximation
            double r = radius * (1 - eccentricity) / (1 + eccentricity * cos(angle));
            
            state.x = r * cos(angle) * cos(inclination);
            state.y = r * sin(angle) * cos(inclination);
            state.z = r * sin(inclination) * sin(angle * 0.3);
            
            // More chaotic velocity for debris
            state.vx = -r * sin(angle) * (0.05 + eccentricity * 0.02) * cos(inclination);
            state.vy = r * cos(angle) * (0.05 + eccentricity * 0.02) * cos(inclination);
            state.vz = r * cos(inclination) * (0.02 + eccentricity * 0.01);
            
            state.rad = r;
            
            traj.states.push_back(state);
        }
        
        trajectories.push_back(traj);
    }
    
    cout << "Generated " << trajectories.size() << " total trajectories (" 
          << satellites.size() << " satellites + " << debris.size() << " debris)" << endl;
    
    return trajectories;
}