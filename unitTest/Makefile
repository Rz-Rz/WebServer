# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -Werror -I$(HOME)/Criterion/include/criterion -I$(HOME)/42/WebServer/inc

# Linker flags
LDFLAGS = -Wl,-rpath=$(HOME)/Criterion/build/src -L$(HOME)/Criterion/build/src -lcriterion

# Source files
SOURCES = ConfigurationParsing.cpp ../src/ConfigurationParser.cpp ../src/ParsingUtils.cpp ../src/Logger.cpp ../src/Server.cpp ../src/Route.cpp ../src/ErrorPageManager.cpp ../src/RouteDebug.cpp

SOURCES_REQHANDLER = ReqHand.cpp ../src/ConfigurationParser.cpp ../src/ParsingUtils.cpp ../src/Logger.cpp ../src/Server.cpp ../src/Route.cpp ../src/ErrorPageManager.cpp ../src/RequestHandler.cpp ../src/HTTPRequestParser.cpp ../src/MultipartFormDataParser.cpp ../src/RouteDebug.cpp ../src/SystemUtils.cpp

SOURCES_UTILS = Utils.cpp ../src/Logger.cpp
# Target binary name
TARGET = crit_test

UTILS = utils

REQHANDLER = req

# Build target
$(TARGET): $(SOURCES)
	$(CXX) -o $(TARGET) $(SOURCES) $(CXXFLAGS) $(LDFLAGS)

$(UTILS): $(SOURCES_UTILS)
	$(CXX) -o $(UTILS) $(SOURCES_UTILS) $(CXXFLAGS) $(LDFLAGS)

$(REQHANDLER): $(SOURCES_REQHANDLER)
	$(CXX) -o $(REQHANDLER) $(SOURCES_REQHANDLER) $(CXXFLAGS) $(LDFLAGS)

# Clean target
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: clean


