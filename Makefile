CXX = g++
CXXFLAGS = -std=c++20 -Wall -Werror -g -Isrc
SRC_DIR = src
OBJ_DIR = obj

# Source files
SOURCES = $(SRC_DIR)/main.cc $(SRC_DIR)/scan.cc $(SRC_DIR)/parse.cc $(SRC_DIR)/type.cc $(SRC_DIR)/gen.cc $(SRC_DIR)/asm.cc
OBJECTS = $(OBJ_DIR)/main.o $(OBJ_DIR)/scan.o $(OBJ_DIR)/parse.o $(OBJ_DIR)/type.o $(OBJ_DIR)/gen.o $(OBJ_DIR)/asm.o
HEADERS = $(SRC_DIR)/scan.h $(SRC_DIR)/parse.h $(SRC_DIR)/type.h $(SRC_DIR)/gen.h $(SRC_DIR)/asm.h $(SRC_DIR)/wlp4data.h

EXEC = wlp4

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXEC)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc $(HEADERS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(EXEC)
