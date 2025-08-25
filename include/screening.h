#ifndef SCREENING_H
#define SCREENING_H

#include "types.h"

// Error codes for screening functions
#define SCREENING_SUCCESS 0
#define SCREENING_ERROR_INVALID_INPUT 1
#define SCREENING_ERROR_INSUFFICIENT_BUFFER 2
#define SCREENING_ERROR_MEMORY_ALLOCATION 3

/**
 * Structure representing a close encounter between two satellites
 */
typedef struct {
    char sat1_id[64];           // ID of first satellite
    char sat2_id[64];           // ID of second satellite
    double time_of_closest_approach; // Time in Julian date
    double min_distance_km;     // Minimum distance in kilometers
    double relative_velocity;   // Relative velocity at closest approach (km/s)
    int severity;              // Severity classification (using Severity enum)
    double collision_probability; // Collision probability proxy [0,1]
} Encounter;

/**
 * Compute 3D Euclidean distance between two position vectors
 * 
 * @param pos1 First position vector [x, y, z] in km
 * @param pos2 Second position vector [x, y, z] in km
 * @return Distance in kilometers
 */
double distance3d(const double* pos1, const double* pos2);

/**
 * Classify encounter severity based on distance thresholds
 * 
 * @param distance Distance in kilometers
 * @return Severity code (NONE, LOW, MEDIUM, HIGH, CRASH)
 */
int classify_severity(double distance);

/**
 * Compute collision probability proxy using logistic function
 * 
 * @param distance Distance in kilometers
 * @param relative_velocity Relative velocity in km/s
 * @return Probability proxy value between 0 and 1
 */
double logistic_probability(double distance, double relative_velocity);

/**
 * Screen for conjunctions between multiple satellites
 * 
 * @param trajectories Array of pointers to state vector arrays for each satellite
 * @param ids Array of satellite ID strings
 * @param sat_count Number of satellites
 * @param max_distance_km Maximum distance threshold for encounters
 * @param out_encounters Output array for detected encounters
 * @param out_count Pointer to store number of encounters found
 * @param max_out Maximum number of encounters that can be stored
 * @return Error code (0 = success, non-zero = error)
 */
int screen_conjunctions(const StateVectorECI** trajectories, const char** ids, 
                       size_t sat_count, double max_distance_km, 
                       Encounter* out_encounters, size_t* out_count, size_t max_out);

/**
 * Sort encounters by collision risk (probability * severity factor)
 * 
 * @param encounters Array of encounters to sort
 * @param count Number of encounters
 */
void sort_encounters_by_risk(Encounter* encounters, size_t count);

/**
 * Sort encounters by time of closest approach
 * 
 * @param encounters Array of encounters to sort
 * @param count Number of encounters
 */
void sort_encounters_by_time(Encounter* encounters, size_t count);

/**
 * Filter encounters by minimum probability threshold
 * 
 * @param encounters Array of encounters (modified in-place)
 * @param count Number of encounters
 * @param min_probability Minimum probability threshold
 * @return Number of encounters remaining after filtering
 */
size_t filter_by_probability(Encounter* encounters, size_t count, double min_probability);

/**
 * Compute relative velocity magnitude between two state vectors
 * 
 * @param state1 First state vector
 * @param state2 Second state vector
 * @return Relative velocity magnitude in km/s
 */
double compute_relative_velocity(const StateVectorECI* state1, const StateVectorECI* state2);

#endif // SCREENING_H
