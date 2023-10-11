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
    void log(Level level, const std::string& message);
    
private:
    std::string getLevelString(Level level);
    std::ofstream logFile;
    std::string getCurrentTime();
    std::string generateLogFilename();

};
