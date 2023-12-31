#include "CgiHandler.hpp"
#include "SystemUtils.hpp"
#include "Logger.hpp"
#include "HTTPResponse.hpp"
#include "ParsingUtils.hpp"
#include "SignalHandler.hpp"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdexcept>
#include <sstream>
#include <fcntl.h>
#include <string.h>

CgiHandler::CgiHandler(const std::string& filePath, const std::string& queryString, int client_fd, Reactor* reactor) : client_fd(client_fd), reactor(reactor), childPid(-1) {
  setCGIEnvironment(queryString);
  int cgiPipeFd = executeCGI(filePath);
  EventHandler::setHandle(cgiPipeFd);
}

void CgiHandler::closeConnection(void) {
	reactor->removeFromInactivityList(EventHandler::getHandle());
	reactor->deregisterHandler(EventHandler::getHandle());
	SystemUtils::closeUtil(getHandle());
	SystemUtils::closeUtil(client_fd);
	delete this;
}

void CgiHandler::handleEvent(uint32_t events) {
  if (events & EPOLLIN) {
    char buffer[1024];
    ssize_t bytesRead;
    int cgiPipeFd = EventHandler::getHandle();
    waitpid(childPid, NULL, 0);
    while ((bytesRead = read(cgiPipeFd, buffer, sizeof(buffer))) > 0) {
      cgiOutputBuffer.write(buffer, bytesRead);
    }

    if (bytesRead == -1) {
      // Handle read error
      Logger::log(ERROR, "Error reading from CGI process" + std::string(strerror(errno)));
      closeConnection();
    }

    if (bytesRead == 0) {
      // End of data or non-blocking read would block, process the output
      std::string output = cgiOutputBuffer.str();
      // Process the output, e.g., send as HTTP response
      Cookie cookie("", "");
      HTTPResponse::sendSuccessResponse("200 OK", "text/html", output, cookie, client_fd);
      closeConnection();
    }
  }
}

CgiHandler::~CgiHandler() {}

void CgiHandler::setCGIEnvironment(const std::string& queryString) {
  if (queryString.empty()) {
    return;
  }
  setenv("QUERY_STRING", queryString.c_str(), 1);
}

int CgiHandler::executeCGI(const std::string& filePath) {
    int pipefd[2];
    pid_t pid;

    // Create a pipe for the child process's output
    if (pipe(pipefd) == -1) {
        throw std::runtime_error("Failed to create pipe");
    }

    // Set the reading end of the pipe to non-blocking
    int flags = fcntl(pipefd[0], F_GETFL, 0);
    fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

    pid = fork();
    childPid = pid;
    if (pid == -1) {
        throw std::runtime_error("Failed to fork process");
    }

    if (pid == 0) {
        // Child process
        SystemUtils::closeUtil(pipefd[0]);          
        dup2(pipefd[1], STDOUT_FILENO);
        SystemUtils::closeUtil(pipefd[1]);
        char* execArgs[2];
        execArgs[0] = const_cast<char*>(filePath.c_str());
        execArgs[1] = NULL;
	if (execve(execArgs[0], execArgs, environ) == -1)
		std::cerr << "Error executing CGI script: " << strerror(errno) << std::endl;
        _exit(EXIT_FAILURE);
    } else {
        // Parent process
        SystemUtils::closeUtil(pipefd[1]);          
        return pipefd[0];  // Return the reading end of the pipe
    }
}
