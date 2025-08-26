#include "simplified_core.h"
#include "types.h"

int main(int argc, char* argv[]) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    
    // Default parameters
    double threshold_meters = 5000.0;
    double step_seconds = 60.0;
    double duration_hours = 24.0;
    
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
    
    
    // Stream conjunctions JSON per timestep (no duplicate screening pass)
    writeTracksJSON(tracks, startEpochMs, startEpochMs + duration_hours * 3600000.0, step_seconds);
    streamConjunctionsJSON(tracks, threshold_meters);
    
    
    return 0;
}
