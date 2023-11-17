# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -Werror -I$(HOME)/Criterion/include/criterion -I$(HOME)/42/WebServer/inc

# Linker flags
LDFLAGS = -Wl,-rpath=$(HOME)/Criterion/build/src -L$(HOME)/Criterion/build/src -lcriterion

# Source files
SOURCES = ConfigurationParsing.cpp ../src/ConfigurationParser.cpp

# Target binary name
TARGET = crit_test

# Build target
$(TARGET): $(SOURCES)
	$(CXX) -o $(TARGET) $(SOURCES) $(CXXFLAGS) $(LDFLAGS)

# Clean target
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: clean


