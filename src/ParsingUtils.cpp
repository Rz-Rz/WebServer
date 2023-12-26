#include "ParsingUtils.hpp"
#include <algorithm>
#include <cctype>
#include <locale>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include "Logger.hpp"
#include <string.h>
#include <unistd.h>


ParsingUtils::ParsingUtils() {}

ParsingUtils::~ParsingUtils() {}

bool ParsingUtils::isNotSpace(char ch) {
    return !std::isspace(static_cast<unsigned char>(ch));
}

// Trim from start (in place)
void ParsingUtils::ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), isNotSpace));
}

// Trim from end (in place)
void ParsingUtils::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), isNotSpace).base(), s.end());
}

// Trim from both ends (in place)
void ParsingUtils::trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

bool ParsingUtils::isRegularFile(const std::string &path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0) {
        return S_ISREG(buffer.st_mode);
    }
    return false;
}

// Trim from both ends (copying)
std::string ParsingUtils::trim_copy(const std::string& s) {
	std::string str = s;
	trim(str);
	return str;
}

bool ParsingUtils::matcher(const std::string& str, const std::string& toFind) {
	std::string trimmedStr = trim_copy(str);
	std::string trimmedToFind = trim_copy(toFind);

	std::string lowerStr = toLower(trimmedStr);
	std::string lowerToFind = toLower(trimmedToFind);

	size_t pos = lowerStr.find(lowerToFind);
	if (pos != std::string::npos) {
		// Check for word boundaries
		if ((pos == 0 || !std::isalnum(lowerStr[pos - 1])) &&
				(pos + lowerToFind.length() == lowerStr.length() || !std::isalnum(lowerStr[pos + lowerToFind.length()]))) {
			return true;
		}
	}
	return false;
}

bool ParsingUtils::simpleMatcher(const std::string& str, const std::string& toFind) {
    std::string trimmedStr = trim_copy(str);
    std::string trimmedToFind = trim_copy(toFind);

    // Convert both strings to lowercase for case-insensitive matching
    std::string lowerStr = toLower(trimmedStr);
    std::string lowerToFind = toLower(trimmedToFind);

    // Check if 'toFind' is a substring of 'str'
    return lowerStr.find(lowerToFind) != std::string::npos;
}



std::string ParsingUtils::toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

void ParsingUtils::toLowerInline(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}


bool ParsingUtils::controlCharacters(const std::string &str)
{
	for (size_t i = 0; i < str.length(); i++) {
		if (str[i] < 32 || str[i] > 126) {
			return true;
		}
	}
	return false;
}

void ParsingUtils::setPrefixString(std::string &str, const std::string &prefix)
{
	str = prefix + str;
}

bool ParsingUtils::doesPathExist(const std::string& path) {
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

bool ParsingUtils::hasReadPermissions(const std::string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0) {
        if (S_ISDIR(buffer.st_mode)) {
            // Check if the path is a directory and if it has read and execute permissions
            // Execute permission is required on a directory to list its contents
            return (buffer.st_mode & S_IXUSR) && (buffer.st_mode & S_IRUSR);
        } else {
            // For regular files, just check read permission
            return (buffer.st_mode & S_IRUSR);
        }
    }
    return false;
}

bool ParsingUtils::hasWritePermissions(const std::string& path) {
	struct stat buffer;
	if (stat(path.c_str(), &buffer) == 0) {
		return (buffer.st_mode & S_IWUSR);
	}
	return false;
}

bool ParsingUtils::doesPathExistAndReadable(const std::string& path) {
  return doesPathExist(path) && hasReadPermissions(path);
}

bool ParsingUtils::isDirectory(const std::string& path) {
  struct stat buffer;
  if (stat(path.c_str(), &buffer) == 0) {
    return S_ISDIR(buffer.st_mode);
  }
  return false;
}

bool ParsingUtils::isValidIPv4(const std::string& host) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, host.c_str(), &(sa.sin_addr));
    return result == 1;
}

bool ParsingUtils::containsIllegalUrlCharacters(const std::string& path) {
  const std::string legalChars = "-_.~/";  // Legal alongside alphanumeric characters

  for (size_t i = 0; i < path.length(); ++i) {
    char ch = path[i];

    if (!std::isalnum(static_cast<unsigned char>(ch)) && legalChars.find(ch) == std::string::npos) {
      return true;  // Found illegal character
    }
  }
  return false;
}

void ParsingUtils::trimAndLower(std::string& str) {
  trim(str);
  toLowerInline(str);
}

bool ParsingUtils::isAbsoluteUrl(const std::string& url) {
    // Common schemes found in absolute URLs
    static const std::string schemes[] = {"http://", "https://", "ftp://", "ftps://", "mailto:", "file:"};
    for (size_t i = 0; i < sizeof(schemes) / sizeof(schemes[0]); ++i) {
        if (url.compare(0, schemes[i].length(), schemes[i]) == 0) {
            return true; // The URL starts with a known scheme
        }
    }
    return false;
}

bool ParsingUtils::containsAlpha(std::string& str) {
  for (size_t i = 0; i < str.length(); ++i) {
    if (std::isalpha(str[i])) {
      return true;
    }
  }
  return false;
}

std::string ParsingUtils::readFile(const std::string& filePath) {
    std::ifstream fileStream(filePath.c_str(), std::ios::in | std::ios::binary);

    if (!fileStream) {
        // Handle the error, could throw an exception or return an error message
        std::cerr << "Error opening file: " << filePath << std::endl;
        return "";
    }

    // Read the entire file content
    std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

    fileStream.close();
    return content;
}

std::vector<std::string> ParsingUtils::getDirectoryContents(const std::string& directoryPath) {
    std::vector<std::string> contents;
    DIR *dir = opendir(directoryPath.c_str()); // Open the directory
    if (dir != NULL) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) { // Read each entry
            std::string fileName = entry->d_name;
            // Optionally, filter out '.' and '..'
            if (fileName != "." && fileName != "..") {
                contents.push_back(fileName);
            }
        }
        closedir(dir); // Close the directory
    } else {
      Logger::log(ERROR, "Error opening directory: " + std::string(strerror(errno)));
      throw std::runtime_error("Error opening directory: " + std::string(strerror(errno)));
    }
    return contents;
}

std::string ParsingUtils::getCurrentWorkingDirectory(void) {
  char buffer[PATH_MAX];
  if (getcwd(buffer, sizeof(buffer)) != NULL) {
    return std::string(buffer);
  } else {
    // Handle error
    return "";
  }
}

bool ParsingUtils::hasWriteAndExecutePermissions(const std::string& path) {
    struct stat st;

    if (stat(path.c_str(), &st) != 0) {
        // Error in retrieving the stat info
        perror("Error getting directory information");
        return false;
    }

    // Check if current user is the owner of the directory
    if (st.st_uid == getuid()) {
        return (st.st_mode & S_IWUSR) && (st.st_mode & S_IXUSR);
    }

    // Check if current user is part of the directory's group
    if (st.st_gid == getgid()) {
        return (st.st_mode & S_IWGRP) && (st.st_mode & S_IXGRP);
    }

    // Check for others
    return (st.st_mode & S_IWOTH) && (st.st_mode & S_IXOTH);
}

bool ParsingUtils::hasExecutePermissions(const std::string &path)
{
  return access(path.c_str(), X_OK) == 0;
}
