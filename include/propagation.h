#ifndef PROPAGATION_H
#define PROPAGATION_H

#include "types.h"

// Error codes for propagation functions
#define PROPAGATION_SUCCESS 0
#define PROPAGATION_ERROR_CONVERGENCE 1
#define PROPAGATION_ERROR_INVALID_INPUT 2
#define PROPAGATION_ERROR_NAN_RESULT 3

/**
 * Main orbital propagation function using simplified SGP4 algorithm
 * 
 * @param elements Pointer to orbital elements structure
 * @param minutes_since_epoch Time offset from epoch in minutes
 * @param out_state Pointer to output state vector in ECI coordinates
 * @return Error code (0 = success, non-zero = error)
 */
int propagate(const OrbitalElements* elements, double minutes_since_epoch, StateVectorECI* out_state);

// Internal helper functions are declared as static in the implementation file

#endif // PROPAGATION_H
