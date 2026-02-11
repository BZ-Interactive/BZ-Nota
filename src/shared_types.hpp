#pragma once
#include <cstdint>
#include <limits>

/// @brief Status type used for UI status bars and file operation results
enum class StatusBarType {
    NORMAL,
    SUCCESS,
    ERROR,
    WARNING
};
