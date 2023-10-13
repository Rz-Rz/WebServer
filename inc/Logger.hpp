#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>

enum Level {
	INFO,
	WARNING,
	ERROR,
	DEBUG
};

class Logger {
public:
    Logger();
    ~Logger();
    static void log(Level level, const std::string& message);
    
private:
    static std::string getLevelString(Level level);
    static std::ofstream logFile;
    static std::string getCurrentTime();
    static std::string generateLogFilename();
};
#endif
