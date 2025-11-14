TARGET := stella
SRC_DIRS := ./src
# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. The shell will incorrectly expand these otherwise, but we want to send the * directly to the find command.
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
COMMON_FLAGS := $(INC_FLAGS) -MMD -MP -std=c++20 -Wall -Wextra -Werror
LDFLAGS = -lasan -fsanitize=address -fno-omit-frame-pointer -fwrapv

RELEASE_CPP_FLAGS := -O2 $(COMMON_CPP_FLAGS) 
LINT_CPP_FLAGS := -O0 $(COMMON_FLAGS) 
DEBUG_CPP_FLAGS := -g3 -O0 $(COMMON_FLAGS) 
PROFILE_CPP_FLAGS := -g -O0 $(COMMON_FLAGS) -fno-inline
VALGRIND_CPP_FLAGS := -g -O2 $(COMMON_FLAGS)

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

RELEASE_OBJS := $(SRCS:%=$(RELEASE_DIR)/%.o)
LINT_OBJS := $(SRCS:%=$(LINT_DIR)/%.o)
DEBUG_OBJS := $(SRCS:%=$(DEBUG_DIR)/%.o)
PROFILE_OBJS := $(SRCS:%=$(PROFILE_DIR)/%.o)
VALGRIND_OBJS := $(SRCS:%=$(VALGRIND_DIR)/%.o)

$(info    RELEASE_OBJS is $(RELEASE_OBJS))
# Commands

run: $(RELEASE_DIR)/$(TARGET)
	./$(RELEASE_DIR)/$(TARGET)

# The final build step.
$(RELEASE_DIR)/$(TARGET): $(RELEASE_OBJS)
	$(CXX) $(RELEASE_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(RELEASE_DIR)/%.cpp.o: %.cpp
	mkdir -p $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(RELEASE_CPP_FLAGS) -c $< -o $@
	
lint: $(LINT_DIR)/$(TARGET)
	./$(LINT_DIR)/$(TARGET)

# The final build step.
$(LINT_DIR)/$(TARGET): $(LINT_OBJS)
	$(CXX) $(LINT_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(LINT_DIR)/%.cpp.o: %.cpp
	mkdir -p $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(LINT_CPP_FLAGS) -c $< -o $@   

debug: $(DEBUG_DIR)/$(TARGET)
	gdb $(DEBUG_DIR)/$(TARGET)

# The final build step.
$(DEBUG_DIR)/$(TARGET): $(DEBUG_OBJS)
	$(CXX) $(DEBUG_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(DEBUG_DIR)/%.cpp.o: %.cpp
	mkdir -p $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(DEBUG_CPP_FLAGS) -c $< -o $@

profile: $(PROFILE_DIR)/$(TARGET)
	./$(PROFILE_DIR)/$(TARGET)
	valgrind --tool=callgrind --callgrind-out-file=callgrind.txt ./$(PROFILE_DIR)/$(TARGET)
	callgrind_annotate callgrind.txt > callgrind_profile.txt

# The final build step.
$(PROFILE_DIR)/$(TARGET): $(PROFILE_OBJS)
	$(CXX) $(PROFILE_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(PROFILE_DIR)/%.cpp.o: %.cpp
	mkdir -p $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(PROFILE_CPP_FLAGS) -c $< -o $@

valgrind: $(VALGRIND_DIR)/$(TARGET)
	valgrind --leak-check=full --track-origins=yes -s --log-file="valgrind.txt" $(VALGRIND_DIR)/$(TARGET) > /dev/null

# The final build step.
$(VALGRIND_DIR)/$(TARGET): $(VALGRIND_DIR)
	$(CXX) $(VALGRIND_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(VALGRIND_DIR)/%.cpp.o: %.cpp
	mkdir -p $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(VALGRIND_CPP_FLAGS) -c $< -o $@



