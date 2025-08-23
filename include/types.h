#ifndef TYPES_H
#define TYPES_H

// Severity levels for encounter classification
#define SEVERITY_COLLISION 0
#define SEVERITY_CRITICAL  1
#define SEVERITY_CLOSE     2
#define SEVERITY_NOMINAL   3

// Two-Line Element data structure
struct TLE {
    char name[24];   // Satellite name
    char line1[70];  // First line of TLE
    char line2[70];  // Second line of TLE
};

// Orbital elements derived from TLE
struct OrbitalElements {
    double epoch;           // Epoch time (Julian date)
    double mean_motion;     // Mean motion (revolutions per day)
    double eccentricity;    // Orbital eccentricity
    double inclination;     // Inclination (radians)
    double raan;           // Right ascension of ascending node (radians)
    double arg_perigee;    // Argument of perigee (radians)
    double mean_anomaly;   // Mean anomaly (radians)
    double bstar;          // BSTAR drag term
    double ndot;           // First derivative of mean motion
    double nddot;          // Second derivative of mean motion
};

// Earth-Centered Inertial state vector
struct StateVectorECI {
    double t;       // Time (Julian date)
    double r[3];    // Position vector (km)
    double v[3];    // Velocity vector (km/s)
};

// Satellite encounter data
struct Encounter {
    char id[64];              // Encounter identifier
    double tca;               // Time of closest approach (Julian date)
    double distance;          // Minimum distance (km)
    double relative_velocity; // Relative velocity at TCA (km/s)
    int severity;             // Severity level (0-3)
    double probability;       // Collision probability
};

// Function declarations
const char* severity_to_string(int severity);

#endif // TYPES_H
