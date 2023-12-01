#include "ParsingUtils.hpp"
#include <algorithm>
#include <cctype>
#include <locale>
#include <sys/stat.h>
#include <arpa/inet.h>

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



std::string ParsingUtils::toLower(std::string& str) {
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
		return (buffer.st_mode & S_IRUSR);
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
