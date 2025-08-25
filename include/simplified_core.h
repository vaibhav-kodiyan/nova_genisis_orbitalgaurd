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
    std::string id;
    bool isDebris;
    std::vector<State> states;
};

struct Encounter {
    std::string aId;
    std::string bId;
    double t;
    double miss_m;
    double rel_mps;
};

// Function declarations
std::vector<Trajectory> propagate_coords_only(
    const std::vector<std::string>& ids,
    const std::vector<bool>& isDebrisFlags,
    double startEpochMs,
    double stepSeconds,
    double durationHours);

std::vector<Encounter> screen_by_threshold(
    const std::vector<Trajectory>& tracks, 
    double threshold_m);
