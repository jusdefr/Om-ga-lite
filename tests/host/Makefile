# Host unit tests for the portable part of Omega Lite custom colors.
#
#   make            build and run the tests
#   make build      build only
#   make clean
#
# These tests need a desktop C++17 compiler and nothing else: the firmware
# glue (Ion storage, escher views) is deliberately excluded.

CXX ?= g++
CUSTOM_COLORS_DIR := ../../apps/settings/custom_colors
BUILD_DIR := ../../build/host-tests
BINARY := $(BUILD_DIR)/custom_colors_tests

WARNINGS := -Wall -Wextra -Werror -Wshadow -Wpedantic
CXXFLAGS ?= -std=c++17 -O2 -g
CPPFLAGS += -I$(CUSTOM_COLORS_DIR) -I.

LOCAL_SOURCES := \
  main.cpp \
  test_color_rgb.cpp \
  test_color_roles.cpp \
  test_color_overrides.cpp \
  test_color_settings.cpp

MODULE_SOURCES := \
  color_rgb.cpp \
  color_roles.cpp \
  color_overrides.cpp \
  color_scheme.cpp \
  color_store.cpp \
  color_settings.cpp

OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(LOCAL_SOURCES:.cpp=.o))) \
           $(addprefix $(BUILD_DIR)/custom_,$(notdir $(MODULE_SOURCES:.cpp=.o)))

.PHONY: all build run clean

all: run

build: $(BINARY)

run: $(BINARY)
	@echo "Running host tests..."
	@$(BINARY)

$(BINARY): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/custom_%.o: $(CUSTOM_COLORS_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(WARNINGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
