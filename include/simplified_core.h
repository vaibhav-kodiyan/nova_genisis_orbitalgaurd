#pragma once
#include "project_includes.h"

// Simplified core data structures
struct State {
    double t;
    double x, y, z;
    double vx, vy, vz;
    double rad;
};

struct Trajectory {
    string id;
    bool isDebris;
    vector<State> states;
};

struct Encounter {
    string aId;
    string bId;
    double t;
    double miss_m;
    double rel_mps;
};

// Function declarations
vector<Trajectory> propagate_coords_only(
    const vector<string>& ids,
    const vector<bool>& isDebrisFlags,
    double startEpochMs,
    double stepSeconds,
    double durationHours);

vector<Encounter> screen_by_threshold(
    const vector<Trajectory>& tracks, 
    double threshold_m);

// JSON serialization helpers
void writeTracksJSON(const vector<Trajectory>& tracks, double startMs, double stopMs, double stepSeconds);
void writeEncountersJSON(const vector<Encounter>& encounters);
