#include "wasm_api.h"
#include "types.h"
#include "propagation.h"
#include "screening.h"
#include "time_utils.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <memory>
#include <limits>

// Forward declarations for functions we need
bool parse_tle_to_elements(const char* name, const char* line1, const char* line2, OrbitalElements* elements);
int propagate_sgp4(const OrbitalElements* elements, double minutes_from_epoch, StateVectorECI* state);

// JSON utility functions
namespace {
    std::string escape_json_string(const std::string& str) {
        std::string escaped;
        for (char c : str) {
            switch (c) {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\b': escaped += "\\b"; break;
                case '\f': escaped += "\\f"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default: escaped += c; break;
            }
        }
        return escaped;
    }

    double unix_ms_to_julian(double unix_ms) {
        // Unix epoch (1970-01-01 00:00:00 UTC) to Julian date
        return (unix_ms / 86400000.0) + 2440587.5;
    }

    double julian_to_unix_ms(double julian) {
        return (julian - 2440587.5) * 86400000.0;
    }

    std::string severity_to_string(int severity) {
        switch (severity) {
            case HIGH: return "High";
            case MEDIUM: return "Medium";
            case LOW: return "Low";
            default: return "Low";
        }
    }

    int classify_severity_by_distance(double distance_m) {
        if (distance_m < 1000.0) return HIGH;
        else if (distance_m < 5000.0) return MEDIUM;
        else if (distance_m < 25000.0) return LOW;
        else return NONE;
    }

    double logistic_pc_proxy(double miss_distance_m) {
        // Logistic function: pc = 1/(1+exp(k*(d - d0)))
        const double k = 0.001;  // steepness parameter
        const double d0 = 2000.0; // inflection point at 2km
        return 1.0 / (1.0 + std::exp(k * (miss_distance_m - d0)));
    }
}

extern "C" {

const char* og_parse_tle(const char* text, int kind) {
    if (!text) return nullptr;
    
    std::string input(text);
    std::istringstream stream(input);
    std::string line;
    std::vector<std::string> lines;
    
    // Read all lines
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    
    std::ostringstream json;
    json << "[";
    
    bool first = true;
    for (size_t i = 0; i + 2 < lines.size(); i += 3) {
        const std::string& name = lines[i];
        const std::string& line1 = lines[i + 1];
        const std::string& line2 = lines[i + 2];
        
        // Validate TLE format
        if (line1.length() >= 69 && line2.length() >= 69 && 
            line1[0] == '1' && line2[0] == '2') {
            
            if (!first) json << ",";
            first = false;
            
            json << "{";
            json << "\"name\":\"" << escape_json_string(name) << "\",";
            json << "\"line1\":\"" << escape_json_string(line1) << "\",";
            json << "\"line2\":\"" << escape_json_string(line2) << "\",";
            json << "\"kind\":\"" << (kind == 0 ? "satellite" : "debris") << "\"";
            json << "}";
        }
    }
    
    json << "]";
    
    std::string result = json.str();
    char* output = (char*)malloc(result.length() + 1);
    if (output) {
        strcpy(output, result.c_str());
    }
    return output;
}

const char* og_compute_simulation(const char* tle_json, double start_ms, double stop_ms, double step_s) {
    if (!tle_json) return nullptr;
    
    // Parse JSON manually (simple parser for known format)
    std::string input(tle_json);
    std::vector<TLE> tles;
    
    // Simple JSON parsing - look for TLE entries
    size_t pos = 0;
    while ((pos = input.find("\"name\":", pos)) != std::string::npos) {
        TLE tle;
        
        // Extract name
        size_t name_start = input.find("\"", pos + 7) + 1;
        size_t name_end = input.find("\"", name_start);
        tle.name = input.substr(name_start, name_end - name_start);
        
        // Extract line1
        size_t line1_start = input.find("\"line1\":\"", name_end) + 9;
        size_t line1_end = input.find("\"", line1_start);
        std::string line1 = input.substr(line1_start, line1_end - line1_start);
        strncpy(tle.line1, line1.c_str(), sizeof(tle.line1) - 1);
        
        // Extract line2
        size_t line2_start = input.find("\"line2\":\"", line1_end) + 9;
        size_t line2_end = input.find("\"", line2_start);
        std::string line2 = input.substr(line2_start, line2_end - line2_start);
        strncpy(tle.line2, line2.c_str(), sizeof(tle.line2) - 1);
        
        tles.push_back(tle);
        pos = line2_end;
    }
    
    double start_julian = unix_ms_to_julian(start_ms);
    double stop_julian = unix_ms_to_julian(stop_ms);
    double step_days = step_s / 86400.0;
    
    std::ostringstream json;
    json << "[";
    
    bool first_track = true;
    for (const auto& tle : tles) {
        if (!first_track) json << ",";
        first_track = false;
        
        // Parse TLE to orbital elements
        OrbitalElements elements;
        if (!parse_tle_to_elements(tle.name.c_str(), tle.line1, tle.line2, &elements)) {
            continue;
        }
        
        json << "{";
        json << "\"id\":\"" << escape_json_string(tle.name) << "\",";
        json << "\"kind\":\"satellite\","; // Default to satellite for now
        json << "\"states\":[";
        
        bool first_state = true;
        for (double t = start_julian; t <= stop_julian; t += step_days) {
            StateVectorECI state;
            double minutes_from_epoch = (t - elements.epoch) * 1440.0; // Convert days to minutes
            
            if (propagate_sgp4(&elements, minutes_from_epoch, &state) == 0) {
                if (!first_state) json << ",";
                first_state = false;
                
                json << "{";
                json << "\"t\":" << julian_to_unix_ms(t) << ",";
                json << "\"r\":[" << state.r[0] << "," << state.r[1] << "," << state.r[2] << "],";
                json << "\"v\":[" << state.v[0] << "," << state.v[1] << "," << state.v[2] << "]";
                json << "}";
            }
        }
        
        json << "]}";
    }
    
    json << "]";
    
    std::string result = json.str();
    char* output = (char*)malloc(result.length() + 1);
    if (output) {
        strcpy(output, result.c_str());
    }
    return output;
}

const char* og_run_analysis(const char* tracks_json, double sync_tol_s) {
    if (!tracks_json) return nullptr;
    
    // Parse tracks JSON and perform conjunction screening
    std::string input(tracks_json);
    std::vector<std::vector<StateVectorECI>> all_tracks;
    std::vector<std::string> track_ids;
    
    // Simple JSON parsing for tracks
    size_t pos = 0;
    while ((pos = input.find("\"id\":", pos)) != std::string::npos) {
        // Extract ID
        size_t id_start = input.find("\"", pos + 5) + 1;
        size_t id_end = input.find("\"", id_start);
        std::string id = input.substr(id_start, id_end - id_start);
        track_ids.push_back(id);
        
        // Extract states
        std::vector<StateVectorECI> states;
        size_t states_start = input.find("\"states\":[", id_end);
        size_t states_end = input.find("]", states_start);
        
        size_t state_pos = states_start;
        while ((state_pos = input.find("\"t\":", state_pos)) != std::string::npos && state_pos < states_end) {
            StateVectorECI state;
            
            // Parse t
            size_t t_start = state_pos + 4;
            size_t t_end = input.find(",", t_start);
            state.t = unix_ms_to_julian(std::stod(input.substr(t_start, t_end - t_start)));
            
            // Parse r array
            size_t r_start = input.find("\"r\":[", t_end) + 5;
            sscanf(input.c_str() + r_start, "%lf,%lf,%lf", &state.r[0], &state.r[1], &state.r[2]);
            
            // Parse v array
            size_t v_start = input.find("\"v\":[", r_start) + 5;
            sscanf(input.c_str() + v_start, "%lf,%lf,%lf", &state.v[0], &state.v[1], &state.v[2]);
            
            states.push_back(state);
            state_pos = input.find("}", v_start);
        }
        
        all_tracks.push_back(states);
        pos = states_end;
    }
    
    // Perform conjunction screening
    std::vector<Encounter> encounters;
    const double max_distance_km = 25.0; // 25 km max screening distance
    
    for (size_t i = 0; i < all_tracks.size(); ++i) {
        for (size_t j = i + 1; j < all_tracks.size(); ++j) {
            const auto& track1 = all_tracks[i];
            const auto& track2 = all_tracks[j];
            
            // Find closest approach
            double min_distance = std::numeric_limits<double>::max();
            double tca = 0;
            double rel_velocity = 0;
            
            for (const auto& state1 : track1) {
                for (const auto& state2 : track2) {
                    if (std::abs(state1.t - state2.t) <= sync_tol_s / 86400.0) {
                        double dist = std::sqrt(
                            std::pow(state1.r[0] - state2.r[0], 2) +
                            std::pow(state1.r[1] - state2.r[1], 2) +
                            std::pow(state1.r[2] - state2.r[2], 2)
                        );
                        
                        if (dist < min_distance) {
                            min_distance = dist;
                            tca = state1.t;
                            
                            // Calculate relative velocity
                            rel_velocity = std::sqrt(
                                std::pow(state1.v[0] - state2.v[0], 2) +
                                std::pow(state1.v[1] - state2.v[1], 2) +
                                std::pow(state1.v[2] - state2.v[2], 2)
                            );
                        }
                    }
                }
            }
            
            if (min_distance <= max_distance_km) {
                Encounter encounter;
                strncpy(encounter.sat1_id, track_ids[i].c_str(), sizeof(encounter.sat1_id) - 1);
                strncpy(encounter.sat2_id, track_ids[j].c_str(), sizeof(encounter.sat2_id) - 1);
                encounter.time_of_closest_approach = tca;
                encounter.min_distance_km = min_distance;
                encounter.relative_velocity = rel_velocity;
                encounter.severity = classify_severity_by_distance(min_distance * 1000); // Convert to meters
                encounter.collision_probability = logistic_pc_proxy(min_distance * 1000);
                
                encounters.push_back(encounter);
            }
        }
    }
    
    // Generate JSON output
    std::ostringstream json;
    json << "{\"encounters\":[";
    
    bool first = true;
    for (const auto& enc : encounters) {
        if (!first) json << ",";
        first = false;
        
        json << "{";
        json << "\"aId\":\"" << escape_json_string(enc.sat1_id) << "\",";
        json << "\"bId\":\"" << escape_json_string(enc.sat2_id) << "\",";
        json << "\"tcaUtc\":" << julian_to_unix_ms(enc.time_of_closest_approach) << ",";
        json << "\"missMeters\":" << (enc.min_distance_km * 1000) << ",";
        json << "\"relSpeedMps\":" << (enc.relative_velocity * 1000) << ",";
        json << "\"pcProxy\":" << enc.collision_probability << ",";
        json << "\"severity\":\"" << severity_to_string(enc.severity) << "\"";
        json << "}";
    }
    
    json << "]}";
    
    std::string result = json.str();
    char* output = (char*)malloc(result.length() + 1);
    if (output) {
        strcpy(output, result.c_str());
    }
    return output;
}

void og_free_string(const char* ptr) {
    if (ptr) {
        free((void*)ptr);
    }
}

} // extern "C"
