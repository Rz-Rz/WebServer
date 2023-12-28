# Makefile for libcurl tester

# Compiler settings
CC = g++
CFLAGS = -Wall -Wextra -I$(HOME)/42/WebServer/inc
LDFLAGS = -lcurl

# Source and executable files
SRC = curl_tester.cpp
OBJ = $(SRC:.cpp=.o)
EXE = libcurl_tester

# Default target
all: $(EXE)

# Rule for building the final executable
$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Rule for compiling source files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

# Clean up build files
clean:
	rm -f $(OBJ) $(EXE)

# PHONY targets
.PHONY: all clean

