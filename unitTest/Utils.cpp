
#include <criterion.h>
#include <regex>
#include "Logger.hpp" // Include your Logger header

// Test for timestamp format
Test(logger, get_current_time_format) {
    std::string currentTime = Logger::getCurrentTime();
    std::regex timeFormatRegex("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}");
    cr_assert(std::regex_match(currentTime, timeFormatRegex), "Current time should match the YYYY-MM-DD HH:MM:SS format.");
}

// Test for timestamp length
Test(logger, get_current_time_length) {
    std::string currentTime = Logger::getCurrentTime();
    cr_assert_eq(currentTime.size(), 19, "Length of the timestamp string should be 19 characters.");
}
