#pragma once
#include "utilities.h"

std::string GetTimeStampStr() {
    // Get the current time as a time_point
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Convert to std::time_t
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;

    // Convert time_t to tm struct safely
    localtime_s(&localTime, &now_c);

    // Create a string stream to hold the formatted time
    std::ostringstream oss;

    // Format the time as "YYYY-MM-DD HH:MM:SS"
    oss << std::put_time(&localTime, "[%Y-%m-%d %H:%M:%S");

    // Add milliseconds
    oss << '.' << std::setw(3) << std::setfill('0') << ms.count()<<"]";

    // Return the formatted time as a string
    return oss.str();
}
