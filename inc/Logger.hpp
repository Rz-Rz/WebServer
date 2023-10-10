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
    Logger(const std::string& filename) : logFile(filename.c_str(), std::ios::app);
    void log(Level level, const std::string& message);
    
private:
    std::string getLevelString(Level level);
    std::ofstream logFile;
};
