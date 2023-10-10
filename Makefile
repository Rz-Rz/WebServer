# Compiler
CXX = c++
GXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Criterion
CRITERION = -I$$HOME/Criterion/include/criterion -Wl,-rpath=$$HOME/Criterion/build/src -L$$HOME/Criterion/build/src -W -lcriterion

# Test sources and objects
TEST_SOURCES = $(wildcard unitTest/*.cpp)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

# Target binary name
TARGET = webserver

# Source files
SOURCES = $(wildcard src/*.cpp) main.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Build target
all: $(TARGET)

# Test target
test: 
	$(GXX) $(CRITERION) $(CXXFLAGS)  -o test_$@ $^
	./test_$@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Clean target
clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(TARGET)

