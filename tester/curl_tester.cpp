#include <iostream>
#include <string>
#include "ParsingUtils.hpp"
#include <curl/curl.h>

// Function to set the resolve option in libcurl
void set_resolve_option(CURL *curl, const std::string& hostname, int port, const std::string& address) {
    std::string resolve_data = hostname + ":" + ParsingUtils::toString(port) + ":" + address;
    struct curl_slist *resolve_list = NULL;
    resolve_list = curl_slist_append(resolve_list, resolve_data.c_str());
    curl_easy_setopt(curl, CURLOPT_RESOLVE, resolve_list);
}

// Function to send HTTP request and return the response code
long send_request(const std::string& url, const std::string& method, const std::string& data = "", const std::string& hostname = "", int port = 0, const std::string& address = "") {
    CURL *curl;
    CURLcode res;
    long response_code = 0;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());

        // Set the resolve option if hostname, port, and address are provided
        if (!hostname.empty() && port != 0 && !address.empty()) {
            set_resolve_option(curl, hostname, port, address);
        }

        // Specific handling for POST and DELETE methods
        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        } else if (method == "DELETE") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }

        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            // Get the HTTP response code
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        }

        // Always cleanup
        curl_easy_cleanup(curl);
    }

    return response_code;
}

// Function to check if the response code matches the expected value
bool check_response(long expected, long actual) {
    return expected == actual;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <base_url>" << std::endl;
        return 1;
    }

    std::string base_url = argv[1];

    long response;
    // Test GET method 
    response = send_request(base_url, "GET", "", "example.com", 8080, "127.0.0.1");
    if (check_response(200, response)) {
        std::cout << "GET Test passed." << std::endl;
    } else {
        std::cout << "GET Test failed. Expected 200, got " << response << std::endl;
    }



    // Example tests
    // Add other tests based on your configuration
    // Example: Test POST method
    // long response = send_request(base_url + "/api", "POST", "data=example", "apisafe.com", 8081, "127.0.0.1");
    // if (check_response(200, response)) {
    //     std::cout << "POST Test passed." << std::endl;
    // } else {
    //     std::cout << "POST Test failed. Expected 200, got " << response << std::endl;
    // }

    // Example: Test DELETE method
    // response = send_request(base_url + "/uploads/test.txt", "DELETE", "", "upload.com", 8083, "127.0.0.1");
    // if (check_response(200, response)) {
    //     std::cout << "DELETE Test passed." << std::endl;
    // } else {
    //     std::cout << "DELETE Test failed. Expected 200, got " << response << std::endl;
    // }

    return 0;
}

