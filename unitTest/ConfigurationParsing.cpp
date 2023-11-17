#include <criterion.h>
#include "ConfigurationParser.hpp"

// Test Suite for isValidRoute
Test(isValidRoute, empty_string) {
    cr_assert_eq(isValidRoute(""), false, "Empty string should return false");
}

Test(isValidRoute, starts_with_slash) {
    cr_assert_eq(isValidRoute("/validRoute"), true, "String starting with '/' should be valid");
}

Test(isValidRoute, does_not_start_with_slash) {
    cr_assert_eq(isValidRoute("invalidRoute"), false, "String not starting with '/' should be invalid");
}

Test(isValidRoute, contains_control_characters) {
    cr_assert_eq(isValidRoute("/invalid\nRoute"), false, "String with control characters should be invalid");
}

Test(isValidRoute, contains_ASCII_127) {
    cr_assert_eq(isValidRoute("/invalidRoute\x7F"), false, "String with ASCII character 127 should be invalid");
}

