#include <criterion.h>
#include "ParsingUtils.hpp"
#include "ConfigurationParser.hpp"
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>

// --------------------------------------- Test Suite for isValidRoute ------------------------------------------
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


//             --------------------- Test Suite for pathExists ---------------------
void createTempFile(const std::string& path) {
    std::ofstream tempFile(path.c_str());
    tempFile << "Temporary content";
    tempFile.close();
}

void deleteFile(const std::string& path) {
    unlink(path.c_str());
}
Test(pathExists, file_exists_and_readable) {
    std::string tempPath = "test_temp_file.txt";
    createTempFile(tempPath);
    cr_assert_eq(pathExists(tempPath), true, "pathExists should return true for existing and readable file");
    deleteFile(tempPath);
}

Test(pathExists, file_does_not_exist) {
    std::string nonExistentPath = "non_existent_file.txt";
    cr_assert_eq(pathExists(nonExistentPath), false, "pathExists should return false for non-existent file");
}

Test(pathExists, file_exists_but_not_readable) {
    std::string unreadablePath = "unreadable_file.txt";
    int fd = open(unreadablePath.c_str(), O_CREAT, 00000);
    if (fd == -1) {
        std::cerr << "Error creating file" << std::endl;
        return;
    }

    if (close(fd) == -1) {
        std::cerr << "Error closing file" << std::endl;
        return; 
    }

    cr_assert_eq(pathExists(unreadablePath), false, "pathExists should return false for existing but unreadable file");

   deleteFile(unreadablePath); 
}

// ------------------------------------ Test Suite for extractServerName ------------------------------------

Test(extractServerName, correct_format) {
    std::string line = "[server:MyServer]";
    cr_assert_str_eq(extractServerName(line).c_str(), "MyServer", "Should correctly extract 'MyServer'");
}

Test(extractServerName, incorrect_prefix) {
    std::string line = "server:MyServer]";
    std::string result = extractServerName(line);
    cr_assert_str_eq(result.c_str(), "DefaultServerName", "Should return default name for incorrect prefix");
}

Test(extractServerName, incorrect_suffix) {
    std::string line = "[server:MyServer";
    std::string result = extractServerName(line);
    cr_assert_str_eq(result.c_str(), "DefaultServerName", "Should return default name for incorrect suffix");
}

Test(extractServerName, too_long_name) {
    std::string longName(254, 'a'); // Create a string of 254 'a's
    std::string line = "[server:" + longName + "]";
    std::string result = extractServerName(line);
    cr_assert_str_eq(result.c_str(), "DefaultServerName", "Should return default name for too long server name");
}

Test(extractServerName, contains_illegal_sequence) {
    std::string line = "[server:My..Server]";
    std::string result = extractServerName(line);
    cr_assert_str_eq(result.c_str(), "DefaultServerName", "Should return default name for illegal sequences");
}

Test(extractServerName, empty_string) {
    std::string line = "";
    std::string result = extractServerName(line);
    cr_assert_str_eq(result.c_str(), "DefaultServerName", "Should return default name for empty string");
}

Test(extractServerName, only_prefix_and_suffix) {
    std::string line = "[server:]";
    std::string result = extractServerName(line);
    cr_assert_str_eq(result.c_str(), "DefaultServerName", "Should return default name for empty server name");
}

// ------------------------------------- InvalidChars -----------------------------
Test(containsInvalidCharacter, only_alphanumeric) {
    cr_assert_eq(containsInvalidCharacter("TestString123"), false, "Alphanumeric string should not contain invalid characters");
}

Test(containsInvalidCharacter, only_safe_characters) {
    cr_assert_eq(containsInvalidCharacter(":/?&=.#_-"), false, "String with only safe characters should not contain invalid characters");
}

Test(containsInvalidCharacter, alphanumeric_and_safe_characters) {
    cr_assert_eq(containsInvalidCharacter("Test:123/Name?&=.-_"), false, "Combination of alphanumeric and safe characters should not contain invalid characters");
}

Test(containsInvalidCharacter, contains_invalid_character) {
    cr_assert_eq(containsInvalidCharacter("Invalid@String!"), true, "String with invalid characters should return true");
}

Test(containsInvalidCharacter, empty_string) {
    cr_assert_eq(containsInvalidCharacter(""), false, "Empty string should not contain invalid characters");
}

Test(containsInvalidCharacter, whitespace_character) {
    cr_assert_eq(containsInvalidCharacter(" "), true, "String with whitespace should return true");
}

Test(containsInvalidCharacter, special_characters) {
    cr_assert_eq(containsInvalidCharacter("*&^%$#@!"), true, "String with special characters should return true");
}

// ---------------------------------------- InvalidPath
Test(isValidRedirect, valid_url_with_slash) {
    cr_assert_eq(isValidRedirect("/path/to/resource"), true, "URL starting with '/' and containing valid characters should be valid");
}

Test(isValidRedirect, valid_url_with_http) {
    cr_assert_eq(isValidRedirect("http://example.com"), true, "URL starting with 'http://' and containing valid characters should be valid");
}

Test(isValidRedirect, valid_url_with_https) {
    cr_assert_eq(isValidRedirect("https://example.com"), true, "URL starting with 'https://' and containing valid characters should be valid");
}

Test(isValidRedirect, invalid_url_with_invalid_characters) {
    cr_assert_eq(isValidRedirect("http://example.com/path?query=<invalid>"), false, "URL with invalid characters should be invalid");
}

Test(isValidRedirect, invalid_url_wrong_pattern) {
    cr_assert_eq(isValidRedirect("ftp://example.com"), false, "URL with wrong starting pattern should be invalid");
}

Test(isValidRedirect, empty_string) {
    cr_assert_eq(isValidRedirect(""), false, "Empty string should be considered an invalid URL");
}

// Additional test for a URL with valid pattern but containing invalid characters
Test(isValidRedirect, valid_pattern_with_invalid_characters) {
    cr_assert_eq(isValidRedirect("https://example.com/invalid|url"), false, "URL with valid pattern but invalid characters should be invalid");
}

// Test with whitespace, which is generally considered invalid in URLs
Test(isValidRedirect, url_with_whitespace) {
    cr_assert_eq(isValidRedirect("https://example.com/invalid url"), false, "URL containing whitespace should be invalid");
}

// ---------------------------------------- isValidRoute ----------------------- 
Test(isValidRoute, valid_route) {
    cr_assert_eq(isValidRoute("/validRoute"), true, "A valid route should return true");
}

Test(isValidRoute, invalid_route_no_start_slash) {
    cr_assert_eq(isValidRoute("invalidRoute"), false, "A route not starting with '/' should return false");
}

Test(isValidRoute, route_with_control_character) {
    std::string route = "/invalidRoute\n";
    cr_assert_eq(isValidRoute(route), false, "A route with control characters should return false");
}

Test(isValidRoute, empty_route) {
    cr_assert_eq(isValidRoute(""), false, "An empty route should return false");
}

Test(isValidRoute, route_with_only_slash) {
    cr_assert_eq(isValidRoute("/"), true, "A route with only '/' should be considered valid");
}

Test(isValidRoute, route_with_valid_characters_after_slash) {
    cr_assert_eq(isValidRoute("/route123"), true, "A route with valid characters after '/' should return true");
}

// Test with route containing higher ASCII values
Test(isValidRoute, route_with_higher_ascii_values) {
    std::string route = "/route\x80\x81";
    cr_assert_eq(isValidRoute(route), true, "A route with higher ASCII values should return true");
}

// ------------------------- Valid IPv4 --------------------------
Test(isValidIPv4, valid_ipv4_address) {
    cr_assert_eq(isValidIPv4("192.168.1.1"), true, "Valid IPv4 address should return true");
}

Test(isValidIPv4, invalid_ipv4_too_few_segments) {
    cr_assert_eq(isValidIPv4("192.168.1"), false, "IPv4 address with too few segments should return false");
}

Test(isValidIPv4, invalid_ipv4_too_many_segments) {
    cr_assert_eq(isValidIPv4("192.168.1.1.1"), false, "IPv4 address with too many segments should return false");
}

Test(isValidIPv4, ipv4_with_non_numeric_characters) {
    cr_assert_eq(isValidIPv4("192.168.a.1"), false, "IPv4 address with non-numeric characters should return false");
}

Test(isValidIPv4, ipv4_segment_out_of_range) {
    cr_assert_eq(isValidIPv4("192.168.256.1"), false, "IPv4 address with a segment out of range should return false");
}

Test(isValidIPv4, ipv4_with_leading_zeros) {
    cr_assert_eq(isValidIPv4("192.168.01.1"), false, "IPv4 address with leading zeros should return false");
}

Test(isValidIPv4, empty_ipv4_address) {
    cr_assert_eq(isValidIPv4(""), false, "Empty IPv4 address should return false");
}

Test(isValidIPv4, non_standard_ipv4_format) {
    cr_assert_eq(isValidIPv4("192.168.1.1/24"), false, "Non-standard IPv4 format should return false");
}
// -------------------------------- Methods
Test(isValidMethod, valid_get_method) {
    cr_assert_eq(isValidMethod("GET"), true, "GET should be a valid method");
}

Test(isValidMethod, valid_post_method) {
    cr_assert_eq(isValidMethod("POST"), true, "POST should be a valid method");
}

Test(isValidMethod, valid_get_post_method) {
    cr_assert_eq(isValidMethod("GET,POST"), true, "GET,POST should be a valid method");
}

Test(isValidMethod, valid_post_get_method) {
    cr_assert_eq(isValidMethod("POST,GET"), true, "POST,GET should be a valid method");
}

Test(isValidMethod, invalid_method) {
    cr_assert_eq(isValidMethod("PUT"), false, "PUT should not be a valid method");
}

Test(isValidMethod, invalid_method_with_comma) {
    cr_assert_eq(isValidMethod("GET,PUT"), false, "GET,PUT should not be a valid method");
}

Test(isValidMethod, method_with_additional_comma) {
    cr_assert_eq(isValidMethod("GET,"), false, "GET, should not be a valid method");
}

Test(isValidMethod, method_with_misplaced_comma) {
    cr_assert_eq(isValidMethod(",GET"), false, ",GET should not be a valid method");
}

Test(isValidMethod, empty_method_string) {
    cr_assert_eq(isValidMethod(""), false, "An empty string should not be a valid method");
}

Test(isValidMethod, method_with_extra_spaces) {
    cr_assert_eq(isValidMethod("GET, POST"), false, "GET, POST with a space should not be a valid method");
}

// ---------------------------------- caseInsensitiveFind ---------------------------
Test(caseInsensitiveFind, substring_found_regardless_of_case) {
    cr_assert_eq(caseInsensitiveFind("Hello World", "world"), true, "Should find 'world' in 'Hello World' irrespective of case");
}

Test(caseInsensitiveFind, substring_not_found) {
    cr_assert_eq(caseInsensitiveFind("Hello World", "test"), false, "Should not find 'test' in 'Hello World'");
}

Test(caseInsensitiveFind, empty_string) {
    cr_assert_eq(caseInsensitiveFind("", "world"), false, "Should not find 'world' in an empty string");
}

Test(caseInsensitiveFind, empty_substring) {
    cr_assert_eq(caseInsensitiveFind("Hello World", ""), true, "An empty substring should always be found");
}

Test(caseInsensitiveFind, substring_at_start) {
    cr_assert_eq(caseInsensitiveFind("Hello World", "Hello"), true, "Should find 'Hello' at the start of 'Hello World'");
}

Test(caseInsensitiveFind, substring_at_end) {
    cr_assert_eq(caseInsensitiveFind("Hello World", "World"), true, "Should find 'World' at the end of 'Hello World'");
}

Test(caseInsensitiveFind, substring_same_as_string) {
    cr_assert_eq(caseInsensitiveFind("Hello World", "Hello World"), true, "Should find 'Hello World' in 'Hello World'");
}

Test(caseInsensitiveFind, mixed_case) {
    cr_assert_eq(caseInsensitiveFind("Hello World", "hElLo WoRlD"), true, "Should find 'hElLo WoRlD' in 'Hello World' irrespective of case");
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
