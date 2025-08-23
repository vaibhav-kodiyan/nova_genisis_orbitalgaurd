#include "types.h"

const char* severity_to_string(int severity) {
    switch (severity) {
        case SEVERITY_COLLISION:
            return "COLLISION";
        case SEVERITY_CRITICAL:
            return "CRITICAL";
        case SEVERITY_CLOSE:
            return "CLOSE";
        case SEVERITY_NOMINAL:
            return "NOMINAL";
        default:
            return "UNKNOWN";
    }
}
