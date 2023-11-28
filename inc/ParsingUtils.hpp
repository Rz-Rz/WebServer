#ifndef PARSING_UTILS_HPP
#define PARSING_UTILS_HPP

#include <string>


class ParsingUtils {
	public: 
		// String Utils
		static void trim(std::string& s);
		static std::string trim_copy(const std::string& s);
		static bool matcher(const std::string& str, const std::string& toFind);
		static std::string toLower(std::string& str);
		static void ltrim(std::string& s);
		static void rtrim(std::string& s);
		static bool isNotSpace(char ch);
		static bool controlCharacters(const std::string& str);
		static void setPrefixString(std::string& str, const std::string& prefix);

		// Path Utils
		static bool doesPathExist(const std::string& path);
		static bool hasReadPermissions(const std::string& path);
		static bool hasWritePermissions(const std::string& path);

	private:
		ParsingUtils();
		~ParsingUtils();
};


#endif
