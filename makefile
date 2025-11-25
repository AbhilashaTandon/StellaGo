TARGET := stella
SRC_DIRS := ./src
TARGET_FLAGS := 19 b 
# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. The shell will incorrectly expand these otherwise, but we want to send the * directly to the find command.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS = $(INC_FLAGS) -MMD -MP
COMMON_FLAGS := -std=c++20 -Wall -Wextra -Werror -lasan -fsanitize=address -fno-omit-frame-pointer -fwrapv
LDFLAGS = -lasan -fsanitize=address -fno-omit-frame-pointer -fwrapv

run: CXXFLAGS := -O2 $(COMMON_FLAGS) 
compile: CXXFLAGS := -O2 $(COMMON_FLAGS) 
lint: CXXFLAGS := -O0 $(COMMON_FLAGS) 
debug: CXXFLAGS := -g3 -O0 $(COMMON_FLAGS) 
profile: CXXFLAGS := -g -O0 $(COMMON_FLAGS) 
valgrind: CXXFLAGS := -g -O2 $(COMMON_FLAGS) 

valgrind: LDFLAGS = -g
profile: LDFLAGS = -g

BUILD_DIR := ./build
RELEASE_DIR = $(BUILD_DIR)/release
LINT_DIR = $(BUILD_DIR)/lint
DEBUG_DIR = $(BUILD_DIR)/debug
PROFILE_DIR = $(BUILD_DIR)/profile
VALGRIND_DIR = $(BUILD_DIR)/valgrind

.PHONY: all debug1 debug2 clean whatever...
clean:
	rm -r $(BUILD_DIR)

OBJS := $(SRCS:%=$(BUILD_DIR)/.*/%.o)
DEPS := $(OBJS:.o=.d)

RELEASE_OBJS := $(SRCS:%=$(RELEASE_DIR)/%.o)
LINT_OBJS := $(SRCS:%=$(LINT_DIR)/%.o)
DEBUG_OBJS := $(SRCS:%=$(DEBUG_DIR)/%.o)
PROFILE_OBJS := $(SRCS:%=$(PROFILE_DIR)/%.o)
VALGRIND_OBJS := $(SRCS:%=$(VALGRIND_DIR)/%.o)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)

# Commands

run: $(RELEASE_DIR)/$(TARGET)
	./$(RELEASE_DIR)/$(TARGET) $(TARGET_FLAGS)

compile: $(RELEASE_DIR)/$(TARGET) 

# The final build step.
$(RELEASE_DIR)/$(TARGET): $(RELEASE_OBJS)
	$(CXX) $(RELEASE_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(RELEASE_DIR)/%.cpp.o: %.cpp
	mkdir -p $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
lint: $(LINT_DIR)/$(TARGET)
	./$(LINT_DIR)/$(TARGET) $(TARGET_FLAGS)

# The final build step.
$(LINT_DIR)/$(TARGET): $(LINT_OBJS) 
	$(CXX) $(LINT_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(LINT_DIR)/%.cpp.o: %.cpp
	mkdir -p $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@   

debug: $(DEBUG_DIR)/$(TARGET)
	gdb $(DEBUG_DIR)/$(TARGET) $(TARGET_FLAGS)

# The final build step.
$(DEBUG_DIR)/$(TARGET): $(DEBUG_OBJS) 
	$(CXX) $(DEBUG_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(DEBUG_DIR)/%.cpp.o: %.cpp
	mkdir -p $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

profile: $(PROFILE_DIR)/$(TARGET) 
	./$(PROFILE_DIR)/$(TARGET) $(TARGET_FLAGS)
	valgrind --tool=callgrind --callgrind-out-file=callgrind.txt ./$(PROFILE_DIR)/$(TARGET) $(TARGET_FLAGS)
	callgrind_annotate callgrind.txt > callgrind_profile.txt

# The final build step.
$(PROFILE_DIR)/$(TARGET): $(PROFILE_OBJS) 
	$(CXX) $(PROFILE_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(PROFILE_DIR)/%.cpp.o: %.cpp
	mkdir -p $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

valgrind: $(VALGRIND_DIR)/$(TARGET)
	valgrind --leak-check=full --track-origins=yes -s --log-file="valgrind.txt" $(VALGRIND_DIR)/$(TARGET) $(TARGET_FLAGS) > /dev/null

# The final build step.
$(VALGRIND_DIR)/$(TARGET): $(VALGRIND_OBJS) 
	$(CXX) $(VALGRIND_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(VALGRIND_DIR)/%.cpp.o: %.cpp
	mkdir -p $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@



