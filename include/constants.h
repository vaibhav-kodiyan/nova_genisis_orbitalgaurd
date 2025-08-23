#ifndef CONSTANTS_H
#define CONSTANTS_H

// Gravitational parameter for Earth (km³/s²)
// Source: IERS Conventions 2010
extern const double MU;

// Earth's equatorial radius (km)
// Source: WGS-84 ellipsoid
extern const double EARTH_RADIUS;

// Earth's second zonal harmonic coefficient (dimensionless)
// Source: EGM2008 gravity model
extern const double J2;

// Mathematical constants
extern const double PI;
extern const double TWO_PI;

// Unit conversion factors
extern const double DEG_TO_RAD;
extern const double RAD_TO_DEG;

#endif // CONSTANTS_H
