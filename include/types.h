#pragma once
// centralized std includes
#include "project_includes.h"

// collision risk levels
enum Severity {
    NONE = 0,   // no risk
    LOW = 1,    // safe distance
    MEDIUM = 2, // getting close
    HIGH = 3,   // high risk
    CRASH = 4   // collision imminent
};

// stores TLE (Two-Line Element) data
struct TLE {
    string name;       // satellite name
    char   line1[130]; // first line of TLE (C-style, 129 chars + '\0')
    char   line2[130]; // second line of TLE (C-style, 129 chars + '\0')
};

// orbital elements for calculations
struct OrbitalElements {
    double semi_major_axis;  // orbit size in kilometers
    double eccentricity;     // shape of orbit (0=circular, <1=elliptical)
    double tilt;            // orbit tilt in radians (was inclination)
    double inclination;     // orbit tilt in radians (alias for compatibility)
    double node;            // where orbit crosses equator in radians (was raan)
    double raan;            // right ascension of ascending node (alias for compatibility)
    double perigee_angle;   // angle to closest approach in radians (was arg_perigee)
    double arg_perigee;     // argument of perigee (alias for compatibility)
    double position;        // current position in orbit in radians (was true_anomaly)
    double time;            // reference time in Julian date (was epoch)
    double epoch;           // reference time in Julian date (alias for compatibility)
    double mean_motion;     // mean motion in revolutions per day
    double mean_anomaly;    // mean anomaly in radians
};


// position and velocity in space
struct StateVectorECI {
    double t;       // time in Julian date
    double r[3];    // position vector in kilometers
    double v[3];    // velocity vector in kilometers per second
};

struct satellite_data{
    string name; //satellite name (will work as an identifier)
    OrbitalElements orbital_data;
    StateVectorECI state_data;
    TLE tle; // original TLE for re-propagation when needed
};

struct satellites_array{
    vector<satellite_data> satellites;
};

struct conjunction_pairs{
    satellite_data sat1;
    satellite_data sat2;
    double distance;
    
};

// functions -
vector<TLE> parseTLEfile(const string &filename);
string severity_to_string(int level);



