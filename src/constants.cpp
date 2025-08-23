#include "constants.h"
#include <cmath>

// Gravitational parameter for Earth (km³/s²)
// Value from IERS Conventions 2010
const double MU = 398600.4418;

// Earth's equatorial radius (km)
// WGS-84 ellipsoid semi-major axis
const double EARTH_RADIUS = 6378.137;

// Earth's second zonal harmonic coefficient
// From EGM2008 gravity model
const double J2 = 0.00108262998905;

// Mathematical constants
const double PI = M_PI;
const double TWO_PI = 2.0 * M_PI;

// Unit conversion factors
const double DEG_TO_RAD = M_PI / 180.0;
const double RAD_TO_DEG = 180.0 / M_PI;
