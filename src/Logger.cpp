#include "../inc/Logger.hpp"
#include <ctime>

Logger::Logger() {
  std::string filename = generateLogFilename();
  logFile.open(filename.c_str(), std::ios::app);
  if(!logFile) {
    std::cerr << "Error opening log file: " << filename << std::endl;
  }
}

Logger::~Logger() {
  if (logFile)
    logFile.close();
}

void Logger::log(Level level, const std::string& message) {
	if (logFile)
		logFile << getCurrentTime() << getLevelString(level) << ": " << message << std::endl;
	if (level == WARNING || level == ERROR)
		std::cerr << getLevelString(level) << ": " << message << std::endl;
}

std::string Logger::getCurrentTime() {
  std::time_t now = std::time(NULL);
  char buf[100] = {0};
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
  return buf;
}

std::string Logger::generateLogFilename() {
  std::time_t now = std::time(NULL);
  char buf[100] = {0};
  std::strftime(buf, sizeof(buf), "LOGFILE_%Y%m%d_%H%M%S", std::localtime(&now));
  return buf;
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
