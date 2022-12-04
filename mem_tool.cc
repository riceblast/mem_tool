#include <stdio.h>
#include <iostream>
#include "pebs_frontend.h"

int main(int argc, char *argv[])
{
	if (argc != 3) {
		dprintf(STDERR_FILENO, "usage: sudo ./mem_tool pid cpuid\n");
		exit(0);
	}		

	int event = ALL_MEM;
	int pid = strtol(argv[1], (char**)NULL, 10);
	int cpuid = strtol(argv[2], (char**)NULL, 10);
	printf("pid: %d\n", pid);
	printf("cpuid: %d\n", cpuid);
	
	PebsFrontEnd pebs_frontend(event, pid, cpuid);
	pebs_frontend.perf_setup();
	pebs_frontend.perf_start();
}
