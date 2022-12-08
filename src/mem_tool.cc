#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "pebs_frontend.h"
#include "sl.h"
#include "data.h"
#include "BloomFilter.h"

int event = ALL_ACCESSES;
int pid;
int cpuid = -1;
int target_page_size = 12; // 2 ^ 12byte, the page size we want to focus
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
	int count = 0; // the total access count of all addr in this window
	int data_count; // the access count of addr in this window
	int hot_threshold = 5;
	int hot_page_count = 0;
	int footprint_page_count = 0;
	long long unsigned int address;

	Data data;
	data.timestamp = 0;
	data.addr = 0;
	CM_SL fre_sketch(WINDOW_SIZE, 10 * 1024 * 1024, 3, 3); // 10M memory
	BloomFilter hot_bf(1000);
	BloomFilter footprint_bf(1000);

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
					address = ps->addr >> target_page_size;

					// update footprint
					if (!footprint_bf.query(data.str)) {
						footprint_page_count += 1;	
						footprint_bf.insert(data.str);
					}

					// insert to CM Sketch
					data.timestamp += 1;
					data.addr = address;
					assert(data.timestamp >= 0);			
					fre_sketch.insert(data);

					// update hot page
					data_count = fre_sketch.query(data);
					if (data_count > hot_threshold && 
						!hot_bf.query(data.str)) {
						hot_page_count += 1;
						hot_bf.insert(data.str);
					//	printf("hot page addr: 0x%llx\n", address);
					}

					// print info and reset status
					if (count >= QUERY_PERIOD) {
						assert(count == QUERY_PERIOD);
						printf("hot: %d footprint: %d (KB)\n",
							hot_page_count << target_page_size >> 10,
							footprint_page_count << target_page_size >> 10);
						count = 0;
						hot_page_count = 0;
						footprint_page_count = 0;

						footprint_bf.clear();
						hot_bf.clear();
					}

					break;
				case PERF_RECORD_THROTTLE:
				case PERF_RECORD_UNTHROTTLE:
					dprintf(STDERR_FILENO, "throttle or unthrottle\n");
					break;
				default:
					dprintf(STDERR_FILENO, "error\n");
			}

			p->data_tail += ph->size;
		}

		//printf("count: %d\n", count);
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
		execv(file_name, argv + 1);
	} else {
		pid = rc;
		pebs_begin();
		process_working_set_size();
	}
}
