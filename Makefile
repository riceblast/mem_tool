CC = gcc
CXX = g++
CFLAGS = -g -Wall
CXXFLAGS = -g -Wall -MMD
LDFLAGS = -pthread -lm

#config src inc
MEM_TOOL_SRC = $(MEM_TOOL_HOME)/src
MEM_TOOL_SRCS := $(shell find $(MEM_TOOL_SRC) -maxdepth 1 -name "*.cc")
FRONTEND_SRC = $(MEM_TOOL_HOME)/src/pebs
FRONTEND_SRCS := $(shell find $(FRONTEND_SRC) -name "*.cc")
SKETCH_SRC = $(MEM_TOOL_HOME)/src/sketch
SKETCH_SRCS := $(shell find $(SKETCH_SRC) -name "*.cc")
MEM_TOOL_INC = $(MEM_TOOL_HOME)/inc

#config build
MEM_TOOL_BUILD_DIR = $(MEM_TOOL_HOME)/build
FRONTEND_OBJS := $(addprefix $(MEM_TOOL_BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(FRONTEND_SRCS)))))
SKETCH_OBJS := $(addprefix $(MEM_TOOL_BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(SKETCH_SRCS)))))
MEM_TOOL_BIN_OBJS := $(addprefix $(MEM_TOOL_BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(MEM_TOOL_SRCS)))))
MEM_TOOL_BUILD_OBJS = $(FRONTEND_OBJS) $(SKETCH_OBJS) $(MEM_TOOL_BIN_OBJS)

#config bin
MEM_TOOL_BIN_DIR = $(MEM_TOOL_HOME)/bin
MEM_TOOL_BIN = $(MEM_TOOL_BIN_DIR)/mem_tool

#config test
MEM_TOOL_TEST_DIR = $(MEM_TOOL_HOME)/test

#all: $(MEM_TOOL_BIN)
all: app

# compile memory trace frontend
$(MEM_TOOL_BUILD_DIR)/%.o: $(FRONTEND_SRC)/%.cc
	@echo "------------compile frontend---------------"
	@echo + CXX "->" MEM_TOOL_HOME/$(shell realpath $< --relative-to $(MEM_TOOL_HOME))
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(MEM_TOOL_INC) -c -o $@ $<

# compile sketch backend
# compile frequency sketch
$(MEM_TOOL_BUILD_DIR)/%.o: $(SKETCH_SRC)/frequency/%.cc
	@echo "------------compile frequency sketch---------------"
	@echo + CXX "->" MEM_TOOL_HOME/$(shell realpath $< --relative-to $(MEM_TOOL_HOME))
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(MEM_TOOL_INC) -c -o $@ $<
# compile membership sketch
$(MEM_TOOL_BUILD_DIR)/%.o: $(SKETCH_SRC)/membership/%.cc
	@echo "------------compile membership sketch---------------"
	@echo + CXX "->" MEM_TOOL_HOME/$(shell realpath $< --relative-to $(MEM_TOOL_HOME))
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(MEM_TOOL_INC) -c -o $@ $<

# compile mem_tool relative file
$(MEM_TOOL_BUILD_DIR)/%.o: $(MEM_TOOL_SRC)/%.cc
	@echo "------------compile mem_tool.o---------------"
	@echo + CXX "->" MEM_TOOL_HOME/$(shell realpath $< --relative-to $(MEM_TOOL_HOME))
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(MEM_TOOL_INC) -c -o $@ $<

# compile the mem_tool 
$(MEM_TOOL_BIN): $(MEM_TOOL_BUILD_OBJS)
	@echo "------------compile mem_tool bin---------------"
	@echo + CXX "->" MEM_TOOL_HOME/$(shell realpath $< --relative-to $(MEM_TOOL_HOME))
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(MEM_TOOL_INC) -o $@ $^

-include $(MEM_TOOL_BUILD_OBJS:.o=.d)
	
#some test file
test_array: $(MEM_TOOL_TEST_DIR)/test_array.c
	@mkdir -p $(MEM_TOOL_BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(MEM_TOOL_BIN_DIR)/test_array $(MEM_TOOL_TEST_DIR)/test_array.c

#compile for memtool and test
app: $(MEM_TOOL_BIN) test_array

clean:
	rm -rf $(MEM_TOOL_BUILD_DIR) $(MEM_TOOL_BIN_DIR)

.PHONY: $(MEM_TOOL_BIN) test_array clean
#.PHONY: $(MEM_TOOL_BUILD_OBJS) $(MEM_TOOL_BIN) perf_test test_array clean
