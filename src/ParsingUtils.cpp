#include "ParsingUtils.hpp"
#include <algorithm>
#include <cctype>
#include <locale>

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
std::string ParsingUtils::trim_copy(std::string& s) {
    trim(s);
    return s;
}

bool ParsingUtils::matcher(std::string& str, std::string& toFind) {
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

std::string ParsingUtils::toLower(std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}
