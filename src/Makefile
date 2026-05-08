CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g
LDFLAGS =

TARGET = petri_core
BUILD_DIR = build
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
RUN_NET ?= ../examples/tof_pn_5s.pn

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Program created: $@"

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) $(RUN_NET)

clean:
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaned"