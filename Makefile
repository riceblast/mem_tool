CC = gcc
CXX = g++
MEM_TOOL_SRC = $(MEM_TOOL_HOME)/src
MEM_TOOL_SRCS = $(shell find $(MEM_TOOL_SRC) -maxdepth 0 -name "*.cc")
FRONTEND_SRC = $(MEM_TOOL_HOME)/src/pebs
FRONTEND_SRCS = $(shell find $(PEBS_SRC) -name "*.cc")
SKETCH_SRC = $(MEM_TOOL_HOME)/src/sketch
SKETCH_SRC = $(shell find $(SKETCH_SRC) -name "*.cc")
MEM_TOOL_INC = $(MEM_TOOL_HOME)/inc

MEM_TOOL_BUILD_DIR = $(MEM_TOOL_HOME)/build
FRONTEND_OBJS = $(addprefix $(MEM_TOOL_BUILD_DIR)/, $(addprefix .o, $(basename $(notdir $(FRONTEND_SRCS)))))
SKETCH_OBJS = $(addprefix $(MEM_TOOL_BUILD_DIR)/, $(addprefix .o, $(basename $(notdir $(SKETCH_SRCS)))))
MEM_TOOL_BUILD_OBJS = $(FRONTEND_OBJS) $(SKETCH_OBJS)

MEM_TOOL_BIN_DIR = $(MEM_TOOL_HOME)/bin
MEM_TOOL_BIN = $(MEM_TOOL_BIN_DIR)/mem_tool

MEM_TOOL_TEST_DIR = $(MEM_TOOL_HOME)/test

CFLAGS = -g -Wall
CXXFLAGS = -g -Wall -MMD
LDFLAGS = -pthread -lm

all: $(MEM_TOOL_BIN)

$(MEM_TOOL_BUILD_DIR)/%.o: $(FRONTEND_SRC)/%.cc
	@echo + CXX "->" MEM_TOOL_HOME/$(shell realpath $< --relative-to $(MEM_TOOL_HOME))
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(MEM_TOOL_INC) -c -o $@ $<

$(MEM_TOOL_BUILD_DIR)/%.o: $(SKETCH_SRC)/%.cc
	@echo + CXX "->" MEM_TOOL_HOME/$(shell realpath $< --relative-to $(MEM_TOOL_HOME))
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(MEM_TOOL_INC) -c -o $@ $<

$(MEM_TOOL_BIN): $(MEM_TOOL_BUILD_OBJS)
	@echo + CXX "->" MEM_TOOL_HOME/$(shell realpath $< --relative-to $(MEM_TOOL_HOME))
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(MEM_TOOL_INC) -c -o $@ $<
	

#mem_tool: pebs_frontend.o mem_tool.o
#	$(CXX) $(CFLAGS) $(LDFLAGS) -o mem_tool mem_tool.o pebs_frontend.o

#mem_tool.o: mem_tool.cc
#	$(CXX) $(CFLAGS) $(LDFLAGS) -o mem_tool.o -c mem_tool.cc


#pebs_frontend.o: pebs_frontend.cc
#	$(CXX) $(CFLAGS) $(LDFLAGS) -o pebs_frontend.o -c pebs_frontend.cc

perf_test: perf_test.cc
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $(MEM_TOOL_TEST_DIR)/perf_test $(MEM_TOOL_TEST_DIR)/perf_test.cc

test_array: test_array.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(MEM_TOOL_TEST_DIR)/test_array $(MEM_TOOL_TEST_DIR)/test_array.c

#clean:
#	rm -f perf_test test_array *.o

.PHONY: perf_test test_array clean mem_tool mem_tool.o
