#include "status.h"

const char *power_status_to_string(PowerStatus status) {
    switch (status) {
        case POWER_STATUS_OK:                 return "Success";
        case POWER_STATUS_ERR_NULL_POINTER:   return "Null pointer";
        case POWER_STATUS_ERR_INVALID_COMMAND:return "Invalid command";
        case POWER_STATUS_ERR_SYSTEM:         return "System call failed";
        case POWER_STATUS_ERR_NO_LOCKER:      return "No screen locker found";
        case POWER_STATUS_ERR_NO_LOGOUT_METHOD: return "No logout method found";
        case POWER_STATUS_ERR_NO_LAUNCHER:    return "No menu launcher found";
        case POWER_STATUS_ERR_PERMISSION_DENIED: return "Permission denied";
        case POWER_STATUS_ERR_IO:             return "I/O error";
        case POWER_STATUS_ERR_UNKNOWN:        return "Unknown error";
        default:                              return "Unrecognised status";
    }
}
