#include "simplified_core.h"
#include "types.h"

int main(int argc, char* argv[]) {
    // Default parameters
    double threshold_meters = 5000.0;
    double step_seconds = 60.0;
    double duration_hours = 24.0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--threshold" && i + 1 < argc) {
            threshold_meters = atof(argv[++i]);
        } else if (string(argv[i]) == "--step" && i + 1 < argc) {
            step_seconds = atof(argv[++i]);
        } else if (string(argv[i]) == "--hours" && i + 1 < argc) {
            duration_hours = atof(argv[++i]);
        }
    }
    
    cout << "Starting simplified orbital simulation..." << endl;
    cout << "Threshold: " << threshold_meters << " meters" << endl;
    cout << "Step: " << step_seconds << " seconds" << endl;
    cout << "Duration: " << duration_hours << " hours" << endl;
    
    // Use deterministic epoch time
    double startEpochMs = 1734979200000.0; // Fixed epoch for deterministic results
    
    // Generate coordinates using simplified propagation
    vector<string> ids; // Will be populated by propagate_coords_only
    vector<bool> isDebrisFlags; // Will be populated by propagate_coords_only
    
    auto tracks = propagate_coords_only(ids, isDebrisFlags, startEpochMs, step_seconds, duration_hours);
    
    if (tracks.empty()) {
        cout << "No satellite tracks generated." << endl;
        return 1;
    }
    
    cout << "Generated " << tracks.size() << " satellite tracks" << endl;
    
    // Screen for encounters using threshold-based detection
    auto encounters = screen_by_threshold(tracks, threshold_meters);
    
    cout << "Found " << encounters.size() << " encounters within " << threshold_meters << "m threshold" << endl;
    
    // Write JSON outputs preserving existing schema
    writeTracksJSON(tracks, startEpochMs, startEpochMs + duration_hours * 3600000.0, step_seconds);
    writeEncountersJSON(encounters);
    
    cout << "JSON files written to tests/ directory" << endl;
    cout << "Simulation complete." << endl;
    
    return 0;
}
