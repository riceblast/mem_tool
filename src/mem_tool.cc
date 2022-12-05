#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "pebs_frontend.h"

int event = ALL_ACCESSES;
int pid;
int cpuid = -1;

char file_name[1000]; // store the elf file path

// run the pebs frontend
void pebs_begin(void) {
	printf("pid: %d\n", pid);
	printf("cpuid: %d\n", cpuid);

	PebsFrontEnd pebs_frontend(event, pid, cpuid);
	pebs_frontend.perf_setup();
	pebs_frontend.perf_start();
}

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		dprintf(STDERR_FILENO, "usage: sudo ./mem_tool command\n");
		exit(1);
	}		

	// TODO 考虑多个参数
	memset(file_name, '\0', sizeof(file_name));
	strcpy(file_name, argv[1]);
	if (access(file_name, F_OK | X_OK) != 0) {
		dprintf(STDERR_FILENO, "file: %s doesn't exist or can't exec\n", file_name);
		exit(1);
	}
	
	int rc = fork();
	if (rc < 0){
		fprintf(stderr, "Fork failed\n");
		exit(1);
	} else if (rc == 0) {
		execv(file_name, argv);
	} else {
		pid = rc;
		pebs_begin();
	}
}
