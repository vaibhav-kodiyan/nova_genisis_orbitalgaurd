#pragma once

// Mathematical constants
#define PI 3.14159265358979323846
#define TWO_PI (2.0 * PI)
#define PI_HALF (PI / 2.0)

// Earth constants
#define EARTH_RADIUS 6371.0  // km
#define MU 398600.4418 // km^3/s^2 (gravitational parameter)
#define MU_EARTH 398600.4418 // km^3/s^2 (gravitational parameter)
#define J2 1.08262668e-3 // J2 perturbation coefficient
#define J2_EARTH 1.08262668e-3 // J2 perturbation coefficient

// Time constants
#define SECONDS_PER_DAY 86400.0
#define MINUTES_PER_DAY 1440.0
#define JULIAN_EPOCH 2451545.0 // J2000.0 epoch

// Unit conversions
#define DEG_TO_RAD (PI / 180.0)
#define RAD_TO_DEG (180.0 / PI)

// Orbital mechanics constants
#define KEPLER_TOLERANCE 1e-10
#define KEPLER_MAX_ITERATIONS 30
