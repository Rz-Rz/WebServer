#include <criterion.h>
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


//                                     --------------------- Test Suite for pathExists ---------------------
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
