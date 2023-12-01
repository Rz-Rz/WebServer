#include <criterion.h>
#include "ParsingUtils.hpp"
#include "ConfigurationParser.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Route.hpp"
#include "assert.h"
#include "internal/assert.h"
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>


// ------------------------------------ Utils ----------------------------------
// Create a directory with specified permissions
bool createTestDirectory(const std::string &path, mode_t permissions) {
    if (mkdir(path.c_str(), permissions) != 0) {
        return false;
    }
    return true;
}


// Remove a directory
bool removeTestDirectory(const std::string &path) {
    if (rmdir(path.c_str()) != 0) {
        return false;
    }
    return true;
}

// Check if a directory exists
bool doesDirectoryExist(const std::string &path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

// Create a directory and a file within it
bool createTestDirectoryAndFile(const std::string &dirPath, const std::string &filePath) {
    if (mkdir(dirPath.c_str(), 0755) != 0) {
        return false;
    }
    std::ofstream outfile(filePath.c_str());
    if (!outfile.is_open()) {
        removeTestDirectory(dirPath);  // Cleanup if file creation fails
        return false;
    }
    outfile << "Error page content" << std::endl;
    outfile.close();
    return true;
}
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

Test(configuration_parser, parse_invalid_route_format) {
    Route route;
    std::string line = "route:/kapouet";
    cr_assert_throw(
        ConfigurationParser::parseRoute(line, route),
        ConfigurationParser::InvalidConfigurationException,
        "Route should be enclosed in square brackets"
        );
}

Test(configuration_parser, parse_route_with_extra_characters) {
    Route route;
    std::string line = "[route:/extra123]";
    ConfigurationParser::parseRoute(line, route);
    cr_assert_str_eq(route.getRoutePath().c_str(), "/extra123", "Route path should handle extra alphanumeric characters");
}

Test(configuration_parser, parse_empty_route_path) {
    Route route;
    std::string line = "[route:]";
    cr_assert_throw(
    ConfigurationParser::parseRoute(line, route),
    ConfigurationParser::InvalidConfigurationException,
    "Route path should not be empty"
        );
}

Test(configuration_parser, parse_route_invalid_characters) {
    Route route;
    std::string line = "[route:/invalid?]";
    cr_assert_throw(
        ConfigurationParser::parseRoute(line, route),
        ConfigurationParser::InvalidConfigurationException,
        "Route path should not allow invalid characters"
        );
}

Test(configuration_parser, parse_route_just_slash) {
    Route route;
    std::string line = "[route:/]";
    ConfigurationParser::parseRoute(line, route);
    cr_assert_str_eq(route.getRoutePath().c_str(), "/", "Route path should handle extra one slash character");
}

// ------------------------------------- Methods ------------------------------------
Test(ConfigurationParser, ParseMethods_ValidGetMethod) {
    Route route;
    std::string line = "methods=GET";
    ConfigurationParser::parseMethods(line, route);
    cr_assert(route.getGetMethod(), "GET method should be set.");
    cr_assert_not(route.getPostMethod(), "POST method should not be set.");
}

Test(ConfigurationParser, ParseMethods_ValidPostMethod) {
    Route route;
    std::string line = "methods=POST";
    ConfigurationParser::parseMethods(line, route);
    cr_assert_not(route.getGetMethod(), "GET method should not be set.");
    cr_assert(route.getPostMethod(), "POST method should be set.");
}

Test(ConfigurationParser, ParseMethods_BothValidMethods) {
    Route route;
    std::string line = "methods=GET POST";
    ConfigurationParser::parseMethods(line, route);
    cr_assert(route.getGetMethod(), "GET method should be set.");
    cr_assert(route.getPostMethod(), "POST method should be set.");
}

Test(ConfigurationParser, ParseMethods_InvalidMethod) {
    Route route;
    std::string line = "methods=PUT";
    cr_assert_throw(
        ConfigurationParser::parseMethods(line, route),
        ConfigurationParser::InvalidConfigurationException,
        "PUT is not a valid method"
        );
}

Test(ConfigurationParser, ParseMethods_InvalidMethodWithValidMethod) {
    Route route;
    std::string line = "methods=GET PUT";
    cr_assert_throw(
        ConfigurationParser::parseMethods(line, route),
        ConfigurationParser::InvalidConfigurationException,
        "PUT is not a valid method"
        );
}

Test(ConfigurationParser, ParseMethods_InvalidMethodWithValidMethod2) {
    Route route;
    std::string line = "methods=PUT GET";
    cr_assert_throw(
        ConfigurationParser::parseMethods(line, route),
        ConfigurationParser::InvalidConfigurationException,
        "PUT is not a valid method"
        );
}

Test(ConfigurationParser, ParseMethods_InvalidMethodWithValidMethod3) {
    Route route;
    std::string line = "methods=POST PUT";
    cr_assert_throw(
        ConfigurationParser::parseMethods(line, route),
        ConfigurationParser::InvalidConfigurationException,
        "PUT is not a valid method"
        );
}

Test(ConfigurationParser, ParseMethods_InvalidMethodWithValidMethod4) {
    Route route;
    std::string line = "methods=PUT POST";
    cr_assert_throw(
        ConfigurationParser::parseMethods(line, route),
        ConfigurationParser::InvalidConfigurationException,
        "PUT is not a valid method"
        );
}

Test(ConfigurationParser, ParseMethods_InvalidMethodWithValidMethod5) {
    Route route;
    std::string line = "methods=PUT POST GET";
    cr_assert_throw(
        ConfigurationParser::parseMethods(line, route),
        ConfigurationParser::InvalidConfigurationException,
        "PUT is not a valid method"
        );
}

Test(ConfigurationParser, ParseMethods_InvalidMethodWithValidMethod6) {
    Route route;
    std::string line = "methods=GET PUT POST";
    cr_assert_throw(
        ConfigurationParser::parseMethods(line, route),
        ConfigurationParser::InvalidConfigurationException,
        "PUT is not a valid method"
        );
}

Test(ConfigurationParser, ParseMethods_MultiplesSpaces) {
    Route route;
    std::string line = "methods=         POST     ";
    ConfigurationParser::parseMethods(line, route);
    cr_assert_not(route.getGetMethod(), "GET method should not be set.");
    cr_assert(route.getPostMethod(), "POST method should be set.");
}

Test(ConfigurationParser, ParseMethods_MultiplesSpaceAndLowerCase) {
    Route route;
    std::string line = "methods=         post     ";
    ConfigurationParser::parseMethods(line, route);
    cr_assert_not(route.getGetMethod(), "GET method should not be set.");
    cr_assert(route.getPostMethod(), "POST method should be set.");
}

Test(ConfigurationParser, ParseMethods_RepeatedMethods) {
    Route route;
    std::string line = "methods=GET GET POST POST";
    ConfigurationParser::parseMethods(line, route);
    cr_assert(route.getGetMethod(), "GET method should be set.");
    cr_assert(route.getPostMethod(), "POST method should be set.");
}

// --------------------------------- Redirect ---------------------------------
Test(ConfigurationParser, ParseRedirect_ValidRedirect) {
    Route route;
    std::string line = "redirect=/redirect";
    ConfigurationParser::parseRedirect(line, route);
    cr_assert_str_eq(route.getRedirectLocation().c_str(), "/redirect", "Redirect location should be /redirect");
    cr_assert(route.getRedirect(), "Has redirect should be set to true");
}

Test(ConfigurationParser, ParseRedirect_ValidRedirectWithTrailingSlash) {
    Route route;
    std::string line = "redirect=/redirect/";
    ConfigurationParser::parseRedirect(line, route);
    cr_assert_str_eq(route.getRedirectLocation().c_str(), "/redirect/", "Redirect location should be /redirect/");
    cr_assert(route.getRedirect(), "Has redirect should be set to true");
}

Test(ConfigurationParser, ParseRedirect_ValidRedirectWithLeadingSpace) {
    Route route;
    std::string line = "redirect= /redirect";
    ConfigurationParser::parseRedirect(line, route);
    cr_assert_str_eq(route.getRedirectLocation().c_str(), "/redirect", "Redirect location should be /redirect");
    cr_assert(route.getRedirect(), "Has redirect should be set to true");
}

Test(ConfigurationParser, ParseRedirect_ValidRedirectWithTrailingSpace) {
    Route route;
    std::string line = "redirect=/redirect ";
    ConfigurationParser::parseRedirect(line, route);
    cr_assert_str_eq(route.getRedirectLocation().c_str(), "/redirect", "Redirect location should be /redirect");
    cr_assert(route.getRedirect(), "Has redirect should be set to true");
}

Test(ConfigurationParser, ParseRedirect_ValidRedirectWithLeadingAndTrailingSpaceAndUppercase) {
    Route route;
    std::string line = "redirect=      /reDiRect      ";
    ConfigurationParser::parseRedirect(line, route);
    cr_assert_str_eq(route.getRedirectLocation().c_str(), "/redirect", "Redirect location should be /redirect");
    cr_assert(route.getRedirect(), "Has redirect should be set to true");
}

Test(ConfigurationParser, missingSlash) {
    Route route;
    std::string line = "redirect=   reDiRect    ";
    ConfigurationParser::parseRedirect(line, route);
    cr_assert_str_eq(route.getRedirectLocation().c_str(), "/redirect", "Redirect location should be /redirect/");
    cr_assert(route.getRedirect(), "Has redirect should be set to true");
}

Test(ConfigurationParser, ParseRedirect_AbsoluteURl) {
    Route route;
    std::string line = "redirect=http://www.google.com";
    ConfigurationParser::parseRedirect(line, route);
    std::string test = route.getRedirectLocation().c_str();
    cr_assert_str_eq(test.c_str(), "http://www.google.com", "Redirect location should be http://www.google.com");
    cr_assert(route.getRedirect(), "Has redirect should be set to true");
}

Test(ConfigurationParser, ParseRedirect_AbsoluteURlWithPathAndQuery) {
    Route route;
    std::string line = "redirect=https://example.com/path?query=param";
    ConfigurationParser::parseRedirect(line, route);
    std::string test = route.getRedirectLocation().c_str();
    cr_assert_str_eq(test.c_str(), "https://example.com/path?query=param", "Redirect location should be https://example.com/path?query=param");
    cr_assert(route.getRedirect(), "Has redirect should be set to true");
}

Test(ConfigurationParser, ParseRedirect_AbsoluteURlWithpathAndPort) {
    Route route;
    std::string line = "redirect=https://example.com:8080/path";
    ConfigurationParser::parseRedirect(line, route);
    std::string test = route.getRedirectLocation().c_str();
    cr_assert_str_eq(test.c_str(), "https://example.com:8080/path", "Redirect location should be https://example.com:8080/path");
    cr_assert(route.getRedirect(), "Has redirect should be set to true");
}

Test(ConfigurationParser, ParseRedirect_empty) {
    Route route;
    std::string line = "redirect=";
    ConfigurationParser::parseRedirect(line, route);
    std::string test = route.getRedirectLocation().c_str();
    cr_assert_str_eq(test.c_str(), "", "Redirect location should be empty");
    cr_assert_not(route.getRedirect(), "Has redirect should be set to false");
}


// -------------------------------------- Root -------------------------------------
// Test for non-existent root path
Test(configuration_parser, parse_root_non_existent) {
    std::string nonExistentPath = "/tmp/non_existent_path";
    removeTestDirectory(nonExistentPath);  // Ensure directory does not exist

    std::string line = "root=" + nonExistentPath;
    Route route;
    ConfigurationParser::parseRoot(line, route);
    cr_assert_eq(route.getRootDirectoryPath(), "/var/www/webserver/", "Should revert to default for non-existent path");
}


// Test for root path without read permissions
Test(configuration_parser, parse_root_no_read_permission) {
    std::string noReadPath = "/tmp/no_read_permission";
    createTestDirectory(noReadPath, 0300); // Write-only permissions

    std::string line = "root=" + noReadPath;
    Route route;
    ConfigurationParser::parseRoot(line, route);
    cr_assert_eq(route.getRootDirectoryPath(), "/var/www/webserver/", "Should revert to default for no read permission");

    removeTestDirectory(noReadPath);
}

// Test for root path without write permissions
Test(configuration_parser, parse_root_no_write_permission) {
    std::string noWritePath = "/tmp/no_write_permission";
    createTestDirectory(noWritePath, 0500); // Read-only permissions

    std::string line = "root=" + noWritePath;
    Route route;
    ConfigurationParser::parseRoot(line, route);
    cr_assert_eq(route.getRootDirectoryPath(), "/var/www/webserver/", "Should revert to default for no write permission");

    removeTestDirectory(noWritePath);
}

// Test for valid root path with proper permissions
Test(configuration_parser, parse_root_valid) {
    std::string validPath = "/tmp/valid_path";
    createTestDirectory(validPath, 0700); // Read-write permissions for owner

    std::string line = "root=" + validPath;
    Route route;
    ConfigurationParser::parseRoot(line, route);
    cr_assert_eq(route.getRootDirectoryPath(), validPath, "Should set the specified valid root path");

    removeTestDirectory(validPath);
}

// Test for root path with relative paths leading outside of server root
Test(configuration_parser, parse_root_invalid_relative_path) {
    std::string invalidPath = "/tmp/../invalid_path";
    createTestDirectory(invalidPath, 0700); // Read-write permissions for owner

    std::string line = "root=" + invalidPath;
    Route route;
    cr_assert_throw(ConfigurationParser::parseRoot(line, route), ConfigurationParser::InvalidConfigurationException, "Should throw exception for invalid relative paths");

    removeTestDirectory(invalidPath);
}

// -------------------------------------- DirectoryListing -------------------------------------

// Test for empty directory listing
Test(configuration_parser, parse_directory_listing_empty) {
    std::string line = "directory_listing=";
    Route route;
    ConfigurationParser::parseDirectoryListing(line, route);
    cr_assert_not(route.getDirectoryListing(), "Should set directory listing to false for empty value");
}

// Test for valid directory listing 'on'
Test(configuration_parser, parse_directory_listing_on) {
    std::string line = "directory_listing=on";
    Route route;
    ConfigurationParser::parseDirectoryListing(line, route);
    cr_assert(route.getDirectoryListing(), "Should set directory listing to true for 'on'");
}

// Test for valid directory listing 'off'
Test(configuration_parser, parse_directory_listing_off) {
    std::string line = "directory_listing=off";
    Route route;
    ConfigurationParser::parseDirectoryListing(line, route);
    cr_assert_not(route.getDirectoryListing(), "Should set directory listing to false for 'off'");
}

// Test for invalid directory listing value
Test(configuration_parser, parse_directory_listing_invalid) {
    std::string line = "directory_listing=invalid_value";
    Route route;
    ConfigurationParser::parseDirectoryListing(line, route);
    cr_assert_not(route.getDirectoryListing(), "Should revert to default (false) for invalid value");
}

// -------------------------------------- DefaultFile -------------------------------------
// Test for empty default file
Test(configuration_parser, parse_default_file_empty) {
    std::string line = "default_file=";
    Route route;
    ConfigurationParser::parseDefaultFile(line, route);
    cr_assert_eq(route.getDefaultFile(), "index.html", "Should set default file to 'index.html' for empty value");
}

// Test for default file with control characters
Test(configuration_parser, parse_default_file_control_chars) {
    std::string line = "default_file=file\x01name"; // Example with a control character
    Route route;
    ConfigurationParser::parseDefaultFile(line, route);
    cr_assert_eq(route.getDefaultFile(), "index.html", "Should revert to 'index.html' for control characters");
}

// Test for default file without leading slash
Test(configuration_parser, parse_default_file_no_leading_slash) {
    std::string line = "default_file=file.html";
    Route route;
    ConfigurationParser::parseDefaultFile(line, route);
    cr_assert_eq(route.getDefaultFile(), "/file.html", "Should prefix default file with '/'");
}

// Test for valid default file
Test(configuration_parser, parse_default_file_valid) {
    std::string line = "default_file=/valid_file.html";
    Route route;
    ConfigurationParser::parseDefaultFile(line, route);
    cr_assert_eq(route.getDefaultFile(), "/valid_file.html", "Should set the specified default file");
}

// Test for unusual but valid default file
Test(configuration_parser, parse_default_file_unusual) {
    std::string line = "default_file=/unusual_file@123.html";
    Route route;
    ConfigurationParser::parseDefaultFile(line, route);
    cr_assert_eq(route.getDefaultFile(), "/unusual_file@123.html", "Should handle unusual but valid default file names");
}

// Test for excessively long default file name
Test(configuration_parser, parse_default_file_long_name) {
    std::string longFileName(1000, 'a'); // 1000 characters long
    std::string line = "default_file=/" + longFileName;
    Route route;
    ConfigurationParser::parseDefaultFile(line, route);
    cr_assert_eq(route.getDefaultFile(), "/" + longFileName, "Should handle long default file names");
}

// -------------------------------------- CgiExtensions -------------------------------------
// Test for empty CGI extensions
Test(configuration_parser, parse_cgi_extensions_empty) {
    std::string line = "cgi_extensions=";
    Route route;
    ConfigurationParser::parseCgiExtensions(line, route);
    cr_assert_not(route.getHasCGI(), "Should disable CGI for empty extensions");
}

// Test for CGI extensions with empty extension
Test(configuration_parser, parse_cgi_extensions_with_empty_extension) {
    std::string line = "cgi_extensions=.php . "; // Includes an empty extension
    Route route;
    ConfigurationParser::parseCgiExtensions(line, route);
    cr_assert_not(route.getHasCGI(), "Should disable CGI for empty extension");
}

// Test for CGI extensions with control characters
Test(configuration_parser, parse_cgi_extensions_with_control_chars) {
    std::string line = "cgi_extensions=.php .py\x01"; // Extension with a control character
    Route route;
    ConfigurationParser::parseCgiExtensions(line, route);
    cr_assert_not(route.getHasCGI(), "Should disable CGI for control characters");
}

// Test for CGI extensions without leading dot
Test(configuration_parser, parse_cgi_extensions_no_leading_dot) {
    std::string line = "cgi_extensions=php py"; // Extensions without a leading dot
    Route route;
    ConfigurationParser::parseCgiExtensions(line, route);
    std::vector<std::string> expected = {".php", ".py"};
    cr_assert_eq(route.getCgiExtensions(), expected, "Should prefix extensions with '.' and set them");
}

// Test for valid CGI extensions
Test(configuration_parser, parse_cgi_extensions_valid) {
    std::string line = "cgi_extensions=.php .html .py";
    Route route;
    ConfigurationParser::parseCgiExtensions(line, route);
    std::vector<std::string> expected = {".php", ".html", ".py"};
    cr_assert_eq(route.getCgiExtensions(), expected, "Should correctly set valid CGI extensions");
}

// Test for multiple CGI extensions
Test(configuration_parser, parse_cgi_extensions_multiple) {
    std::string line = "cgi_extensions=.php .html .py .js";
    Route route;
    ConfigurationParser::parseCgiExtensions(line, route);
    std::vector<std::string> expected = {".php", ".html", ".py", ".js"};
    cr_assert_eq(route.getCgiExtensions(), expected, "Should correctly parse and set multiple CGI extensions");
}

// Test for unusual but valid CGI extensions
Test(configuration_parser, parse_cgi_extensions_unusual) {
    std::string line = "cgi_extensions=.custom_ext@123";
    Route route;
    ConfigurationParser::parseCgiExtensions(line, route);
    std::vector<std::string> expected = {".custom_ext@123"};
    cr_assert_eq(route.getCgiExtensions(), expected, "Should handle unusual but valid CGI extensions");
}

// ------------------------------------ AllowFileUpload --------------------------------
// Test for empty allow file upload
Test(configuration_parser, parse_allow_file_upload_empty) {
    std::string line = "allow_file_upload=";
    Route route;
    ConfigurationParser::parseAllowFileUpload(line, route);
    cr_assert_not(route.getAllowFileUpload(), "Should set allow file upload to false for empty value");
}

// Test for valid allow file upload 'on'
Test(configuration_parser, parse_allow_file_upload_on) {
    std::string line = "allow_file_upload=on";
    Route route;
    ConfigurationParser::parseAllowFileUpload(line, route);
    cr_assert(route.getAllowFileUpload(), "Should set allow file upload to true for 'on'");
}

// Test for valid allow file upload 'off'
Test(configuration_parser, parse_allow_file_upload_off) {
    std::string line = "allow_file_upload=off";
    Route route;
    ConfigurationParser::parseAllowFileUpload(line, route);
    cr_assert_not(route.getAllowFileUpload(), "Should set allow file upload to false for 'off'");
}

// Test for invalid allow file upload value
Test(configuration_parser, parse_allow_file_upload_invalid) {
    std::string line = "allow_file_upload=invalid_value";
    Route route;
    ConfigurationParser::parseAllowFileUpload(line, route);
    cr_assert_not(route.getAllowFileUpload(), "Should set allow file upload to false for invalid value");
}

// Blindspot tests
// Test for non-standard but interpretable values
Test(configuration_parser, parse_allow_file_upload_interpretable_values) {
    std::string lineTrue = "allow_file_upload=yes";
    std::string lineFalse = "allow_file_upload=no";
    Route routeTrue, routeFalse;
    ConfigurationParser::parseAllowFileUpload(lineTrue, routeTrue);
    ConfigurationParser::parseAllowFileUpload(lineFalse, routeFalse);
    cr_assert_not(routeTrue.getAllowFileUpload(), "Non-standard 'yes' value should not be interpreted as 'on'");
    cr_assert_not(routeFalse.getAllowFileUpload(), "Non-standard 'no' value should not be interpreted as 'off'");
}

// Test for values with leading/trailing spaces
Test(configuration_parser, parse_allow_file_upload_with_spaces) {
    std::string line = "allow_file_upload= on ";
    Route route;
    ConfigurationParser::parseAllowFileUpload(line, route);
    cr_assert(route.getAllowFileUpload(), "Should handle values with leading/trailing spaces");
}

// ------------------------------- upload location ----------------------------
Test(configuration_parser, parse_upload_location_control_chars) {
  std::string line = "upload_location=/var/www\x01/uploads";
  Route route;
  cr_assert_throw(ConfigurationParser::parseUploadLocation(line, route), ConfigurationParser::InvalidConfigurationException, "Should throw an exception for control characters");
}

Test(configuration_parser, parse_upload_location_empty) {
    std::string line = "upload_location=";
    Route route;
    ConfigurationParser::parseUploadLocation(line, route);
    cr_assert_eq(route.getUploadLocation(), "/var/www/webserver/uploads/", "Should set default upload location for empty value");
}

// Test for upload location without leading slash
Test(configuration_parser, parse_upload_location_no_leading_slash) {
    std::string line = "upload_location=var/www/uploads";
    Route route;
    ConfigurationParser::parseUploadLocation(line, route);
    cr_assert_eq(route.getUploadLocation(), "/var/www/uploads", "Should prefix upload location with '/'");
}

// Test for valid upload location
Test(configuration_parser, parse_upload_location_valid) {
    std::string line = "upload_location=/var/www/uploads";
    Route route;
    ConfigurationParser::parseUploadLocation(line, route);
    cr_assert_eq(route.getUploadLocation(), "/var/www/uploads", "Should set the specified upload location");
}

// Blindspot tests
// Test for upload location with spaces
Test(configuration_parser, parse_upload_location_with_spaces) {
    std::string line = "upload_location=/var/www/ upload spaces ";
    Route route;
    ConfigurationParser::parseUploadLocation(line, route);
    cr_assert_eq(route.getUploadLocation(), "/var/www/ upload spaces", "Should handle upload locations with spaces");
}

// Test for unusual but valid upload location
Test(configuration_parser, parse_upload_location_unusual) {
    std::string line = "upload_location=/var/www/custom@123/uploads";
    Route route;
    ConfigurationParser::parseUploadLocation(line, route);
    cr_assert_eq(route.getUploadLocation(), "/var/www/custom@123/uploads", "Should handle unusual but valid upload locations");
}

// Test for overly long upload location path
Test(configuration_parser, parse_upload_location_long_path) {
    std::string longPath(1000, 'a'); // 1000 characters long
    std::string line = "upload_location=/" + longPath;
    Route route;
    ConfigurationParser::parseUploadLocation(line, route);
    cr_assert_eq(route.getUploadLocation(), "/" + longPath, "Should handle overly long upload location paths");
}

// -  


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

// ------------------------------- Server checks ------------------------------
// ------------------------------- host parsing -------------------------------
// Test for empty host
Test(configuration_parser, parse_host_empty) {
    std::string line = "host=";
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parseHost(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for empty host");
}

// Test for valid IPv4 address
Test(configuration_parser, parse_host_valid_ipv4) {
    std::string line = "host=192.168.1.1";
    Server serverConfig;
    ConfigurationParser::parseHost(line, serverConfig);
    cr_assert_eq(serverConfig.getHost(), "192.168.1.1", "Should set a valid IPv4 address");
}

// Test for invalid IPv4 address
Test(configuration_parser, parse_host_invalid_ipv4) {
    std::string line = "host=256.256.256.256";
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parseHost(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for invalid IPv4 address");
}

// Blindspot tests
// Test for IPv4 address with additional characters
Test(configuration_parser, parse_host_ipv4_with_additional_chars) {
    std::string line = "host=192.168.1.1abc";
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parseHost(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for IPv4 address with additional characters");
}

// Test for IPv4 address with leading/trailing spaces
Test(configuration_parser, parse_host_ipv4_with_spaces) {
    std::string line = "host= 192.168.1.1 ";
    Server serverConfig;
    ConfigurationParser::parseHost(line, serverConfig);
    cr_assert_eq(serverConfig.getHost(), "192.168.1.1", "Should set a valid IPv4 address and trim spaces");
}

// Test for IPv4 address in alternative formats
Test(configuration_parser, parse_host_ipv4_alternative_format) {
    std::string line = "host=0300.0250.01.01"; // Octal representation
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parseHost(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for IPv4 address in alternative format");
}

// ------------------------------- port parsing -------------------------------

// Test for empty port
Test(configuration_parser, parse_port_empty) {
    std::string line = "port=";
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parsePort(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for empty port");
}

// Test for valid port
Test(configuration_parser, parse_port_valid) {
    std::string line = "port=8080";
    Server serverConfig;
    ConfigurationParser::parsePort(line, serverConfig);
    cr_assert_eq(serverConfig.getPort(), 8080, "Should set a valid port");
}

// Test for port out of range (too high)
Test(configuration_parser, parse_port_too_high) {
    std::string line = "port=65536";
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parsePort(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for port too high");
}

// Test for port out of range (negative)
Test(configuration_parser, parse_port_negative) {
    std::string line = "port=-1";
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parsePort(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for negative port");
}

// Blindspot tests
// Test for non-numeric port
Test(configuration_parser, parse_port_non_numeric) {
    std::string line = "port=abc";
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parsePort(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for non-numeric port");
}

// Test for port with leading zeros
Test(configuration_parser, parse_port_leading_zeros) {
    std::string line = "port=00080";
    Server serverConfig;
    ConfigurationParser::parsePort(line, serverConfig);
    cr_assert_eq(serverConfig.getPort(), 80, "Should correctly interpret port with leading zeros");
}

// Test for port with spaces
Test(configuration_parser, parse_port_with_spaces) {
    std::string line = "port= 8080 ";
    Server serverConfig;
    ConfigurationParser::parsePort(line, serverConfig);
    cr_assert_eq(serverConfig.getPort(), 8080, "Should handle port numbers with spaces");
}

// Test for port with additional characters
Test(configuration_parser, parse_port_with_additional_chars) {
    std::string line = "port=8080abc";
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parsePort(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for port with additional characters");
}

// ------------------------------ server name parsing ------------------------------
// Test for valid server name
Test(configuration_parser, parse_server_name_valid) {
    std::string line = "[server:ValidServerName]";
    Server serverConfig;
    ConfigurationParser::parseServerName(line, serverConfig);
    cr_assert_eq(serverConfig.getServerName(), "ValidServerName", "Should set the correct server name");
}

// Test for invalid server name (wrong format)
Test(configuration_parser, parse_server_name_wrong_format) {
    std::string line = "server:InvalidFormat";
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parseServerName(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for wrong format");
}

// Test for too long server name
Test(configuration_parser, parse_server_name_too_long) {
    std::string longName(300, 'a'); // 300 characters long
    std::string line = "[server:" + longName + "]";
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parseServerName(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for too long name");
}

// Blindspot tests
// Test for server name with consecutive dots
Test(configuration_parser, parse_server_name_consecutive_dots) {
    std::string line = "[server:Invalid..Name]";
    Server serverConfig;
    cr_assert_throw(ConfigurationParser::parseServerName(line, serverConfig), ConfigurationParser::InvalidConfigurationException, "Should throw exception for consecutive dots");
}

// Test for server name with leading/trailing spaces
Test(configuration_parser, parse_server_name_with_spaces) {
    std::string line = "[server:  ServerWithSpaces  ]";
    Server serverConfig;
    ConfigurationParser::parseServerName(line, serverConfig);
    cr_assert_eq(serverConfig.getServerName(), "ServerWithSpaces", "Should handle server names with spaces correctly");
}

// ------------------------------ Error Page Parsing ------------------------------
Test(configuration_parser, parse_error_pages_valid_with_existing_path) {
    const std::string tempDir = "/tmp/test_error_pages/";
    const std::string tempFilePath = tempDir + "default.html";
    if (createTestDirectoryAndFile(tempDir, tempFilePath) == false) // Create test directory and file
        std::cout << "CRITICAL: Could not create test directory and file for error page parsing unit tests" << std::endl;

    std::string line = "error_page=404,500," + tempFilePath;
    Server serverConfig;
    ConfigurationParser::parseErrorPages(line, serverConfig);

    cr_assert_eq(serverConfig.getErrorPage(404), tempFilePath, "404 error page should be set correctly");
    cr_assert_eq(serverConfig.getErrorPage(500), tempFilePath, "500 error page should be set correctly");

    removeTestDirectory(tempDir); // Clean up
}

// Test for invalid format (missing '=')
Test(configuration_parser, parse_error_pages_missing_equals) {
    std::string line = "error_page 404 /errors/default.html";
    Server serverConfig;
    ConfigurationParser::parseErrorPages(line, serverConfig);
    cr_assert_eq(serverConfig.getErrorPage(404), "", "Default error page should be used for incorrect format");
}

// Test for non-numeric error codes
Test(configuration_parser, parse_error_pages_non_numeric_code) {
    std::string line = "error_page=abc,/errors/default.html";
    Server serverConfig;
    ConfigurationParser::parseErrorPages(line, serverConfig);
    cr_assert_eq(serverConfig.getErrorPage(0), "", "Default error page should be used for non-numeric error code");
}

// Test for error code out of range
Test(configuration_parser, parse_error_pages_code_out_of_range) {
    std::string line = "error_page=700,/errors/default.html";
    Server serverConfig;
    ConfigurationParser::parseErrorPages(line, serverConfig);
    cr_assert_eq(serverConfig.getErrorPage(700), "", "Default error page should be used for out-of-range error code");
}

// Test for mixed valid and invalid error codes
Test(configuration_parser, parse_error_pages_mixed_valid_invalid) {
    createTestDirectoryAndFile("/tmp/test_error_pages/", "/tmp/test_error_pages/default.html");
    std::string line = "error_page=404,abc,500,/tmp/test_error_pages/default.html";
    Server serverConfig;
    ConfigurationParser::parseErrorPages(line, serverConfig);
    cr_assert_eq(serverConfig.getErrorPage(404), "/tmp/test_error_pages/default.html", "Valid 404 error page should be set");
    cr_assert_eq(serverConfig.getErrorPage(500), "/tmp/test_error_pages/default.html", "Valid 500 error page should be set");
    cr_assert_eq(serverConfig.getErrorPage(0), "", "Default error page should be used for invalid 'abc' error code");
    removeTestDirectory("/tmp/test_error_pages/");
}

// -------------------------------- Max body size --------------------------
// Test for valid max body size without suffix
Test(configuration_parser, parse_client_max_body_size_valid_no_suffix) {
    std::string line = "client_max_body_size=5000";
    Server serverConfig;
    ConfigurationParser::parseClientMaxBodySize(line, serverConfig);
    cr_assert_eq(serverConfig.getMaxClientBodySize(), 5000LL, "Max body size should be set correctly without suffix");
}

// Test for valid max body size with suffix 'K'
Test(configuration_parser, parse_client_max_body_size_valid_with_K_suffix) {
    std::string line = "client_max_body_size=5K";
    Server serverConfig;
    ConfigurationParser::parseClientMaxBodySize(line, serverConfig);
    cr_assert_eq(serverConfig.getMaxClientBodySize(), 5LL * 1024, "Max body size should be set correctly with 'K' suffix");
}

// Test for invalid non-numeric value
Test(configuration_parser, parse_client_max_body_size_invalid_non_numeric) {
    std::string line = "client_max_body_size=abc";
    Server serverConfig;
    ConfigurationParser::parseClientMaxBodySize(line, serverConfig);
    cr_assert_eq(serverConfig.getMaxClientBodySize(), 1000000LL, "Default max body size should be used for non-numeric value");
}

// Test for unsupported suffix
Test(configuration_parser, parse_client_max_body_size_unsupported_suffix) {
    std::string line = "client_max_body_size=500G";
    Server serverConfig;
    ConfigurationParser::parseClientMaxBodySize(line, serverConfig);
    cr_assert_eq(serverConfig.getMaxClientBodySize(), 1000000LL, "Default max body size should be used for unsupported suffix");
}

// Test for empty value
Test(configuration_parser, parse_client_max_body_size_empty) {
    std::string line = "client_max_body_size=";
    Server serverConfig;
    ConfigurationParser::parseClientMaxBodySize(line, serverConfig);
    cr_assert_eq(serverConfig.getMaxClientBodySize(), 1000000LL, "Default max body size should be used for empty value");
}

// Test for extremely large value
Test(configuration_parser, parse_client_max_body_size_extremely_large) {
    std::string line = "client_max_body_size=999999999999";  // An extremely large value
    Server serverConfig;
    ConfigurationParser::parseClientMaxBodySize(line, serverConfig);

    // Check if the function reverts to the default value when faced with an extremely large value
    const long long defaultMaxBodySize = 1000000;  // Default value (1 MB in bytes)
    cr_assert_eq(serverConfig.getMaxClientBodySize(), defaultMaxBodySize, "Default max body size should be used for extremely large value");
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


// ------------------------------------ simpleMatcher --------------------------------

Test(ParsingUtils, matcher_with_actual_directives) {
    std::string str = "[server:example.com]";
    std::string toFind = "[server:";
    cr_assert_eq(ParsingUtils::simpleMatcher(str, toFind), true, "Should match '[server:' in the string");
}


// ------------------------------------ trimAndLower --------------------------------
Test(ParsingUtils, trimAndLower_with_leading_and_trailing_spaces) {
    std::string str = "  TEST  ";
    ParsingUtils::trimAndLower(str);
    cr_assert_str_eq(str.c_str(), "test", "trimAndLower should trim and lowercase the string");
}

Test(ParsingUtils, trimAndLower_with_leading_and_trailing_spaces_and_non_alphanumeric_characters) {
    std::string str = "  TEST!  ";
    ParsingUtils::trimAndLower(str);
    cr_assert_str_eq(str.c_str(), "test!", "trimAndLower should trim and lowercase the string");
}

// ---------------------------------- isAbsoluteUrl --------------------------------
Test(ParsingUtils, isAbsoluteUrl_with_http) {
    std::string url = "http://www.google.com";
    cr_assert_eq(ParsingUtils::isAbsoluteUrl(url), true, "Should return true for an absolute url");
}

Test(ParsingUtils, isAbsoluteUrl_with_https) {
    std::string url = "https://www.google.com";
    cr_assert_eq(ParsingUtils::isAbsoluteUrl(url), true, "Should return true for an absolute url");
}
