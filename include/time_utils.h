#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Gregorian calendar date/time structure
 */
typedef struct {
    int year;        // Year (e.g., 2023)
    int month;       // Month (1-12)
    int day;         // Day of month (1-31)
    int hour;        // Hour (0-23)
    int minute;      // Minute (0-59)
    double second;   // Second with fractional part (0.0-59.999...)
} gregorian_time_t;

/**
 * Convert Gregorian calendar time to Julian date
 * @param greg Gregorian time structure
 * @return Julian date (days since J2000.0 epoch)
 */
double gregorian_to_julian(const gregorian_time_t* greg);

/**
 * Convert Julian date to Gregorian calendar time
 * @param jd Julian date
 * @param greg Output Gregorian time structure
 */
void julian_to_gregorian(double jd, gregorian_time_t* greg);

/**
 * Get current time in Gregorian format
 * @param greg Output Gregorian time structure
 */
void get_current_gregorian_time(gregorian_time_t* greg);

/**
 * Parse ISO 8601 date string to Gregorian time
 * Supports formats: "YYYY-MM-DD", "YYYY-MM-DDTHH:MM:SS", "YYYY-MM-DDTHH:MM:SS.sss"
 * @param iso_string ISO 8601 formatted date string
 * @param greg Output Gregorian time structure
 * @return 0 on success, non-zero on error
 */
int parse_iso8601_to_gregorian(const char* iso_string, gregorian_time_t* greg);

/**
 * Format Gregorian time to ISO 8601 string
 * @param greg Gregorian time structure
 * @param buffer Output buffer (must be at least 32 characters)
 * @param buffer_size Size of output buffer
 * @return Number of characters written, or negative on error
 */
int format_gregorian_to_iso8601(const gregorian_time_t* greg, char* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif // TIME_UTILS_H
