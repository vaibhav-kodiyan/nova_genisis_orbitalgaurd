#include "time_utils.h"
#include "constants.h"
#include <cmath>
#include <ctime>
#include <cstring>
#include <cstdio>
#include <cstdlib>

// Convert Gregorian calendar time to Julian date
double gregorian_to_julian(const gregorian_time_t* greg) {
    if (!greg) return 0.0;
    
    int year = greg->year;
    int month = greg->month;
    int day = greg->day;
    
    // Adjust for January and February
    if (month <= 2) {
        year -= 1;
        month += 12;
    }
    
    // Julian day number calculation (Gregorian calendar)
    int a = year / 100;
    int b = 2 - a + (a / 4);
    
    double jd = floor(365.25 * (year + 4716)) + 
                floor(30.6001 * (month + 1)) + 
                day + b - 1524.5;
    
    // Add time of day
    double time_fraction = (greg->hour + greg->minute / 60.0 + greg->second / 3600.0) / 24.0;
    jd += time_fraction;
    
    return jd;
}

// Convert Julian date to Gregorian calendar time
void julian_to_gregorian(double jd, gregorian_time_t* greg) {
    if (!greg) return;
    
    // Extract integer and fractional parts
    double jd_int = floor(jd + 0.5);
    double jd_frac = jd + 0.5 - jd_int;
    
    // Convert to Gregorian calendar
    int a = (int)(jd_int + 32044);
    int b = (4 * a + 3) / 146097;
    int c = a - (146097 * b) / 4;
    int d = (4 * c + 3) / 1461;
    int e = c - (1461 * d) / 4;
    int m = (5 * e + 2) / 153;
    
    greg->day = e - (153 * m + 2) / 5 + 1;
    greg->month = m + 3 - 12 * (m / 10);
    greg->year = 100 * b + d - 4800 + m / 10;
    
    // Convert fractional day to hours, minutes, seconds
    double time_of_day = jd_frac * 24.0;
    greg->hour = (int)time_of_day;
    
    double remaining_minutes = (time_of_day - greg->hour) * 60.0;
    greg->minute = (int)remaining_minutes;
    
    greg->second = (remaining_minutes - greg->minute) * 60.0;
}

// Get current time in Gregorian format
void get_current_gregorian_time(gregorian_time_t* greg) {
    if (!greg) return;
    
    time_t now = time(nullptr);
    struct tm* utc_tm = gmtime(&now);
    
    if (utc_tm) {
        greg->year = utc_tm->tm_year + 1900;
        greg->month = utc_tm->tm_mon + 1;
        greg->day = utc_tm->tm_mday;
        greg->hour = utc_tm->tm_hour;
        greg->minute = utc_tm->tm_min;
        greg->second = utc_tm->tm_sec;
    } else {
        // Fallback to epoch if gmtime fails
        memset(greg, 0, sizeof(gregorian_time_t));
        greg->year = 1970;
        greg->month = 1;
        greg->day = 1;
    }
}

// Parse ISO 8601 date string to Gregorian time
int parse_iso8601_to_gregorian(const char* iso_string, gregorian_time_t* greg) {
    if (!iso_string || !greg) return -1;
    
    // Initialize structure
    memset(greg, 0, sizeof(gregorian_time_t));
    
    // Try different ISO 8601 formats
    int parsed = 0;
    
    // Format: YYYY-MM-DDTHH:MM:SS.sss
    parsed = sscanf(iso_string, "%d-%d-%dT%d:%d:%lf", 
                   &greg->year, &greg->month, &greg->day,
                   &greg->hour, &greg->minute, &greg->second);
    if (parsed == 6) return 0;
    
    // Format: YYYY-MM-DDTHH:MM:SS
    parsed = sscanf(iso_string, "%d-%d-%dT%d:%d:%d", 
                   &greg->year, &greg->month, &greg->day,
                   &greg->hour, &greg->minute, (int*)&greg->second);
    if (parsed == 6) return 0;
    
    // Format: YYYY-MM-DD HH:MM:SS (space separator)
    parsed = sscanf(iso_string, "%d-%d-%d %d:%d:%lf", 
                   &greg->year, &greg->month, &greg->day,
                   &greg->hour, &greg->minute, &greg->second);
    if (parsed == 6) return 0;
    
    // Format: YYYY-MM-DD
    parsed = sscanf(iso_string, "%d-%d-%d", 
                   &greg->year, &greg->month, &greg->day);
    if (parsed == 3) return 0;
    
    return -1; // Parse failed
}

// Format Gregorian time to ISO 8601 string
int format_gregorian_to_iso8601(const gregorian_time_t* greg, char* buffer, size_t buffer_size) {
    if (!greg || !buffer || buffer_size < 32) return -1;
    
    // Check for fractional seconds
    if (greg->second == floor(greg->second)) {
        // No fractional seconds
        return snprintf(buffer, buffer_size, "%04d-%02d-%02dT%02d:%02d:%02d",
                       greg->year, greg->month, greg->day,
                       greg->hour, greg->minute, (int)greg->second);
    } else {
        // Include fractional seconds
        return snprintf(buffer, buffer_size, "%04d-%02d-%02dT%02d:%02d:%06.3f",
                       greg->year, greg->month, greg->day,
                       greg->hour, greg->minute, greg->second);
    }
}
