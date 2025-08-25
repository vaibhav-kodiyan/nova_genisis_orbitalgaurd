#include "simplified_core.h"
#include "types.h"

// JSON serialization functions preserving existing schema
static void writeTracksJSON(const std::vector<Trajectory>& tracks, double startMs, double stopMs, double stepSeconds) {
    std::ofstream jf("tests/coordinates.json");
    jf << "{\n";
    jf << "  \"timestamp_minutes\": " << std::fixed << std::setprecision(6) << (stopMs - startMs) / 60000.0 << ",\n";
    jf << "  \"satellites\": [\n";
    
    // Use final state for each satellite to maintain compatibility
    for (size_t i = 0; i < tracks.size(); ++i) {
        const auto& track = tracks[i];
        if (!track.states.empty()) {
            const auto& finalState = track.states.back();
            jf << "    {\n";
            jf << "      \"name\": \"" << track.id << "\",\n";
            jf << std::fixed << std::setprecision(6);
            jf << "      \"position_km\": [" << finalState.x << ", " << finalState.y << ", " << finalState.z << "],\n";
            jf << "      \"velocity_km_s\": [" << finalState.vx << ", " << finalState.vy << ", " << finalState.vz << "]\n";
            jf << "    }" << (i + 1 < tracks.size() ? ",\n" : "\n");
        }
    }
    jf << "  ]\n";
    jf << "}\n";
}

static void writeEncountersJSON(const std::vector<Encounter>& encounters) {
    std::ofstream jf("tests/conjunctions.json");
    jf << "{\n";
    jf << "  \"timestamp_minutes\": " << std::fixed << std::setprecision(6) << 1440.0 << ",\n";
    jf << "  \"conjunction_pairs\": [\n";
    
    for (size_t k = 0; k < encounters.size(); ++k) {
        const auto& enc = encounters[k];
        jf << "    {\n";
        jf << "      \"sat1\": { \"name\": \"" << enc.aId << "\", \"position_km\": [0, 0, 0] },\n";
        jf << "      \"sat2\": { \"name\": \"" << enc.bId << "\", \"position_km\": [0, 0, 0] },\n";
        jf << "      \"distance_km\": " << std::fixed << std::setprecision(6) << enc.miss_m / 1000.0 << "\n";
        jf << "    }" << (k + 1 < encounters.size() ? ",\n" : "\n");
    }
    jf << "  ]\n";
    jf << "}\n";
}

int main(int argc, char* argv[]) {
    // Default parameters
    double threshold_meters = 5000.0;
    double step_seconds = 60.0;
    double duration_hours = 24.0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--threshold" && i + 1 < argc) {
            threshold_meters = std::atof(argv[++i]);
        } else if (std::string(argv[i]) == "--step" && i + 1 < argc) {
            step_seconds = std::atof(argv[++i]);
        } else if (std::string(argv[i]) == "--hours" && i + 1 < argc) {
            duration_hours = std::atof(argv[++i]);
        }
    }
    
    std::cout << "Starting simplified orbital simulation..." << std::endl;
    std::cout << "Threshold: " << threshold_meters << " meters" << std::endl;
    std::cout << "Step: " << step_seconds << " seconds" << std::endl;
    std::cout << "Duration: " << duration_hours << " hours" << std::endl;
    
    // Use deterministic epoch time
    double startEpochMs = 1734979200000.0; // Fixed epoch for deterministic results
    
    // Generate coordinates using simplified propagation
    std::vector<std::string> ids; // Will be populated by propagate_coords_only
    std::vector<bool> isDebrisFlags; // Will be populated by propagate_coords_only
    
    auto tracks = propagate_coords_only(ids, isDebrisFlags, startEpochMs, step_seconds, duration_hours);
    
    if (tracks.empty()) {
        std::cout << "No satellite tracks generated." << std::endl;
        return 1;
    }
    
    std::cout << "Generated " << tracks.size() << " satellite tracks" << std::endl;
    
    // Screen for encounters using threshold-based detection
    auto encounters = screen_by_threshold(tracks, threshold_meters);
    
    std::cout << "Found " << encounters.size() << " encounters within " << threshold_meters << "m threshold" << std::endl;
    
    // Write JSON outputs preserving existing schema
    writeTracksJSON(tracks, startEpochMs, startEpochMs + duration_hours * 3600000.0, step_seconds);
    writeEncountersJSON(encounters);
    
    std::cout << "JSON files written to tests/ directory" << std::endl;
    std::cout << "Simulation complete." << std::endl;
    
    return 0;
}
