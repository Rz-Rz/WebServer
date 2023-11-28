#include <criterion.h>
#include "ParsingUtils.hpp"
#include "ConfigurationParser.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Route.hpp"
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>

// ------------------------------------ ConfigurationParser --------------------------------
// ------------------------------------- Routes ------------------------------------

Test(configuration_parser, parse_valid_route) {
    Route route;
    std::string line = "[route:/kapouet]";
    ConfigurationParser::parseRoute(line, route);
    cr_assert_str_eq(route.getRoutePath().c_str(), "/kapouet", "Route path should be /kapouet");
}

Test(configuration_parser, parse_valid_route_different_path) {
    Route route;
    std::string line = "[route:/api/v1]";
    ConfigurationParser::parseRoute(line, route);
    cr_assert_str_eq(route.getRoutePath().c_str(), "/api/v1", "Route path should be /api/v1");
}

Test(configuration_parser, parse_route_trailing_slash) {
    Route route;
    std::string line = "[route:/trailing/]";
    ConfigurationParser::parseRoute(line, route);
    cr_assert_str_eq(route.getRoutePath().c_str(), "/trailing/", "Route path should be /trailing/");
}

Test(configuration_parser, parse_invalid_route_format, .expected = ConfigurationParser::InvalidConfigurationException) {
    Route route;
    std::string line = "route:/kapouet"; // Missing brackets
    ConfigurationParser::parseRoute(line, route);
}





// ------------------------------------ ParsingUtils --------------------------------
Test(ParsingUtils, isNotSpace_with_space) {
    cr_assert_eq(ParsingUtils::isNotSpace(' '), false, "isNotSpace should return false for a space character");
}

Test(ParsingUtils, isNotSpace_with_non_space) {
    cr_assert_eq(ParsingUtils::isNotSpace('a'), true, "isNotSpace should return true for a non-space character");
}

Test(ParsingUtils, ltrim_no_leading_spaces) {
    std::string str = "test";
    ParsingUtils::ltrim(str);
    cr_assert_str_eq(str.c_str(), "test", "ltrim should leave string unchanged when there are no leading spaces");
}

Test(ParsingUtils, ltrim_with_leading_spaces) {
    std::string str = "   test";
    ParsingUtils::ltrim(str);
    cr_assert_str_eq(str.c_str(), "test", "ltrim should remove all leading spaces");
}

Test(ParsingUtils, rtrim_no_trailing_spaces) {
    std::string str = "test";
    ParsingUtils::rtrim(str);
    cr_assert_str_eq(str.c_str(), "test", "rtrim should leave string unchanged when there are no trailing spaces");
}

Test(ParsingUtils, rtrim_with_trailing_spaces) {
    std::string str = "test   ";
    ParsingUtils::rtrim(str);
    cr_assert_str_eq(str.c_str(), "test", "rtrim should remove all trailing spaces");
}

// -------------------------------- Matcher --------------------------------
Test(ParsingUtils, matcher_whole_word) {
    std::string str = "This is a test string";
    std::string toFind = "test";
    cr_assert_eq(ParsingUtils::matcher(str, toFind), true, "Should find 'test' as a whole word in the string");
}

Test(ParsingUtils, matcher_not_found) {
    std::string str = "This is a sample string";
    std::string toFind = "test";
    cr_assert_eq(ParsingUtils::matcher(str, toFind), false, "Should not find 'test' in the string");
}

Test(ParsingUtils, matcher_part_of_larger_word) {
    std::string str = "This is a contest string";
    std::string toFind = "test";
    cr_assert_eq(ParsingUtils::matcher(str, toFind), false, "Should not match 'test' as part of 'contest'");
}

Test(ParsingUtils, matcher_with_whitespace) {
    std::string str = "  test string  ";
    std::string toFind = "  test  ";
    cr_assert_eq(ParsingUtils::matcher(str, toFind), true, "Should match 'test' despite leading/trailing whitespaces");
}

Test(ParsingUtils, matcher_at_start) {
    std::string str = "test string";
    std::string toFind = "test";
    cr_assert_eq(ParsingUtils::matcher(str, toFind), true, "Should match 'test' at the start of the string");
}

Test(ParsingUtils, matcher_at_end) {
    std::string str = "string test";
    std::string toFind = "test";
    cr_assert_eq(ParsingUtils::matcher(str, toFind), true, "Should match 'test' at the end of the string");
}

Test(ParsingUtils, matcher_same_as_string) {
    std::string str = "test";
    std::string toFind = "test";
    cr_assert_eq(ParsingUtils::matcher(str, toFind), true, "Should match when 'toFind' is the same as 'str'");
}

Test(ParsingUtils, matcher_empty_string) {
    std::string str = "";
    std::string toFind = "test";
    cr_assert_eq(ParsingUtils::matcher(str, toFind), false, "Should not match when 'str' is empty");
}

Test(ParsingUtils, matcher_empty_toFind) {
    std::string str = "This is a test string";
    std::string toFind = "";
    cr_assert_eq(ParsingUtils::matcher(str, toFind), false, "Should not match when 'toFind' is empty");
}

Test(ParsingUtils, matcher_with_non_alphanumeric_delimiters) {
    std::string str = "An example; test= works well.";
    std::string toFind = "test";
    cr_assert_eq(ParsingUtils::matcher(str, toFind), true, "Should match 'test' even when delimited by non-alphanumeric characters like '='");
}
