#include <criterion.h>
#include "RequestHandler.hpp"


// Tests
Test(request_handler_tests, standard_file_path) {
    RequestHandler handler;

    std::string str = handler.extractDirectoryPath("/static/www/xxx");
    cr_assert_str_eq(str.c_str(), "/static/www/xxx", "Failed to extract directory from standard file path.");
}

Test(request_handler_tests, no_directory_separator) {
    RequestHandler handler;
    std::string str = handler.extractDirectoryPath("executable");
    cr_assert_str_eq(str.c_str(), "executable", "Failed to handle file path with no directory separator.");
}

Test(request_handler_tests, ends_with_directory_separator) {
    RequestHandler handler;
    std::string str = handler.extractDirectoryPath("/usr/local/bin/");
    cr_assert_str_eq(str.c_str(), "/usr/local/bin", "Failed to handle file path that ends with a directory separator.");
}

Test(request_handler_tests, multiple_directory_separators) {
    RequestHandler handler;
    std::string str = handler.extractDirectoryPath("/usr/local/bin/executable/index.html");
    cr_assert_str_eq(str.c_str(), "/usr/local/bin/executable", "Failed to handle file path with multiple directory separators.");
}

Test(request_handler_tests, root_directory_file) {
    RequestHandler handler;
    std::string str = handler.extractDirectoryPath("/dir");
    cr_assert_str_eq(str.c_str(), "/dir", "Failed to handle file path in root directory.");
}

Test(request_handler_tests, empty_string) {
    RequestHandler handler;
    std::string str = handler.extractDirectoryPath("");
    cr_assert_str_eq(str.c_str(), "", "Failed to handle empty string.");
}

Test(request_handler_tests, with_file_and_query_string) {
    RequestHandler handler;
    std::string str = handler.extractDirectoryPath("/dir/index.html?query=string");
    cr_assert_str_eq(str.c_str(), "/dir", "Failed to handle file path with query string.");
}
