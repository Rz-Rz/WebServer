#include "../inc/Logger.hpp"

Logger::Logger(const std::string& filename) : logFile(filename.c_str()) {}

void Logger::log(Level level, const std::string& message) {
	if (logFile)
		logFile << getLevelString(level) << ": " << message << std::endl;
	if (level == WARNING || level == ERROR)
		std::cerr << getLevelString(level) << ": " << message << std::endl;
}

std::string Logger::getLevelString(Level level) {
	switch (level) {
		case INFO:
			return "INFO";
		case WARNING:
			return "WARNING";
		case ERROR:
			return "ERROR";
		case DEBUG:
			return "DEBUG";
		default:
			return "UNKNOWN";
	}
}
