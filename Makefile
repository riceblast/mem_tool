CC = gcc
CXX = g++
CFLAGS = -g -Wall
LDFLAGS = -pthread -lm
PROG = perf_test
SRCS = perf_test.c test_array.c

all: mem_tool

mem_tool: pebs_frontend.o mem_tool.o
	$(CXX) $(CFLAGS) $(LDFLAGS) -o mem_tool mem_tool.o pebs_frontend.o

mem_tool.o: mem_tool.cc
	$(CXX) $(CFLAGS) $(LDFLAGS) -o mem_tool.o -c mem_tool.cc

perf_test: perf_test.cc
	$(CXX) $(CFLAGS) $(LDFLAGS) -o perf_test perf_test.cc

pebs_frontend.o: pebs_frontend.cc
	$(CXX) $(CFLAGS) $(LDFLAGS) -o pebs_frontend.o -c pebs_frontend.cc

test_array: test_array.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o test_array test_array.c

clean:
	rm -f perf_test test_array *.o

.PHONY: perf_test test_array clean mem_tool mem_tool.o
