# Compiler
CXX = c++

# Compiler flags
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinc

# Target binary name
TARGET = webserver

# Source files
SOURCES = $(wildcard src/*.cpp) main.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Build target
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Clean target
clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(TARGET)

