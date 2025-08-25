#ifndef WASM_API_H
#define WASM_API_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * WASM-specific API functions for orbital debris tracking
 * These functions return JSON strings for easy marshaling to/from JavaScript
 */

/**
 * Parse TLE text into JSON array of {name,line1,line2,kind}
 * @param text TLE text content
 * @param kind Body kind: 0=satellite, 1=debris
 * @return JSON string with parsed TLE entries (caller must free)
 */
const char* og_parse_tle(const char* text, int kind);

/**
 * Propagate all bodies over time window
 * @param tle_json JSON array of TLE entries from og_parse_tle
 * @param start_ms Start time in milliseconds since Unix epoch
 * @param stop_ms Stop time in milliseconds since Unix epoch  
 * @param step_s Time step in seconds
 * @return JSON array of Track {id,kind,states:[{t,r[3],v[3]}]} (caller must free)
 */
const char* og_compute_simulation(const char* tle_json, double start_ms, double stop_ms, double step_s);

/**
 * Screen conjunctions and return analysis results
 * @param tracks_json JSON array of tracks from og_compute_simulation
 * @param sync_tol_s Time synchronization tolerance in seconds
 * @return JSON {encounters: [...]} with encounter data (caller must free)
 */
const char* og_run_analysis(const char* tracks_json, double sync_tol_s);

/**
 * Free memory allocated by WASM API functions
 * @param ptr Pointer returned by og_parse_tle, og_compute_simulation, or og_run_analysis
 */
void og_free_string(const char* ptr);

#ifdef __cplusplus
}
#endif

#endif // WASM_API_H
