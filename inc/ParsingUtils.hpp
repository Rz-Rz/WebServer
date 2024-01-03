#ifndef PARSING_UTILS_HPP
#define PARSING_UTILS_HPP

#include <string>
#include <vector>
#include <sstream>
#include <fcntl.h>
#include <errno.h>


class ParsingUtils {
	public: 
		// String Utils
		static void trim(std::string& s);
		static std::string trim_copy(const std::string& s);
		static bool matcher(const std::string& str, const std::string& toFind);
		static std::string toLower(const std::string& str);
    static void toLowerInline(std::string& str);
		static void ltrim(std::string& s);
		static void rtrim(std::string& s);
		static bool isNotSpace(char ch);
		static bool controlCharacters(const std::string& str);
		static void setPrefixString(std::string& str, const std::string& prefix);
    static bool isValidIPv4(const std::string& host);
    static bool containsIllegalUrlCharacters(const std::string& url);
    static void trimAndLower(std::string& str);
    static bool containsAlpha(std::string& str);
    static bool simpleMatcher(const std::string& str, const std::string& toFind);

		// Path Utils
		static bool doesPathExist(const std::string& path);
		static bool hasReadPermissions(const std::string& path);
		static bool hasWritePermissions(const std::string& path);
    static bool hasExecutePermissions(const std::string& path);
    static bool isDirectory(const std::string& path);
    static bool isRegularFile(const std::string& path);
    static bool doesPathExistAndReadable(const std::string& path);
    static bool hasWriteAndExecutePermissions(const std::string& path);
    static std::string readFile(const std::string& filePath);
    static std::vector<std::string> getDirectoryContents(const std::string& directoryPath);
    static std::string getCurrentWorkingDirectory(void);
    static std::string getWebservRoot(void);
    static std::string removeFinalSlash(const std::string& path);
    static bool isFdClosed(int fd) {
    return (fcntl(fd, F_GETFD) == -1 && errno == EBADF);
}

    // url Utils
    static bool isAbsoluteUrl(const std::string& url);

    //template
    template <typename T>
      static std::string toString(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }


	private:
		ParsingUtils();
		~ParsingUtils();
};


#endif
