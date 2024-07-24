# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -pedantic -g -fsanitize=address

# Directories
SRC_DIR = .
OBJ_DIR = .

# Source files
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/CPos.cpp $(SRC_DIR)/ExprElement.cpp $(SRC_DIR)/CustomExpressionBuilder.cpp $(SRC_DIR)/CSpreadsheet.cpp

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Include directories
INCLUDES = -I./

# Libraries and paths
LIBS = -L./x86_64-linux-gnu -lexpression_parser

# Executable name
TARGET = excel

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LIBS)

# Compile .cpp files to .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)

.PHONY: all clean