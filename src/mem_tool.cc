#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include "pebs_frontend.h"

int event = ALL_ACCESSES;
int pid;
int cpuid = -1;
struct perf_event_mmap_page *buffer;

char file_name[1000]; // store the elf file path

// run the pebs frontend
void pebs_begin(void) 
{
	printf("pid: %d\n", pid);
	printf("cpuid: %d\n", cpuid);

	PebsFrontEnd pebs_frontend(event, pid, cpuid);
	pebs_frontend.perf_setup();
	buffer = pebs_frontend.get_buffer();
	//pebs_frontend.perf_start();
}

// periodically print the working set size
void process_working_set_size(void) 
{	
	int count = 0;

	for (;;) {
		struct perf_event_mmap_page *p = buffer;
		char *pbuf = (char *)p + p->data_offset; // goto the data part of buffer

		__sync_synchronize();

		if(p->data_head == p->data_tail) 
			continue;

		while(p->data_tail != p->data_head) {
			struct perf_event_header *ph = (struct perf_event_header *)(pbuf + (p->data_tail % p->data_size));
			struct perf_sample *ps;

			switch(ph->type) {
				case PERF_RECORD_SAMPLE:
					count +=1;
					ps = (struct perf_sample*)ph;
					assert(ps != NULL);
					printf("addr: 0x%llx\n", ps->addr);
					break;
				case PERF_RECORD_THROTTLE:
				case PERF_RECORD_UNTHROTTLE:
					printf("throttle or unthrottle\n");
					break;
				default:
					printf("error\n");
			}

			p->data_tail += ph->size;
		}

		printf("count: %d\n", count);
	}
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
		process_working_set_size();
	}
}
