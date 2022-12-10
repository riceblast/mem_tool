#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
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
FILE *fp = NULL;

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
	double hot_threshold_rate = 0.03; // 2:8
	int hot_page_count = 0;
	int footprint_page_count = 0;
	long long unsigned int address;

	Data data;
	data.timestamp = 0;
	data.addr = 0;
	CM_SL fre_sketch(WINDOW_SIZE, 30 * 1024 * 1024, 3, 3); // 30M memory
	BloomFilter hot_bf(1000000, 2);
	BloomFilter footprint_bf(1000000, 2);

	struct timespec cur_time = {0,0}; 
	clock_gettime(CLOCK_MONOTONIC, &cur_time);
	uint64_t last_second_phase = cur_time.tv_sec;
	uint64_t cur_second_phase = cur_time.tv_sec;
	uint64_t first_nano_step = cur_time.tv_sec * (int)1e9 + cur_time.tv_nsec;
	uint64_t cur_nano_stamp;

	uint64_t last_record_count = 1; // last period count, used to predict current period
	uint64_t cur_record_count = 1;

	for (;;) {
		struct perf_event_mmap_page *p = buffer;
		char *pbuf = (char *)p + p->data_offset; // goto the data part of buffer

		//clock_gettime(CLOCK_MONOTONIC, &cur_time);
		//printf("nsec: %lu\n", cur_time.tv_nsec);

		__sync_synchronize();

		if(p->data_head == p->data_tail) 
			continue;

		while(p->data_tail != p->data_head) {
			struct perf_event_header *ph = (struct perf_event_header *)(pbuf + (p->data_tail % p->data_size));
			struct perf_sample *ps;

			switch(ph->type) {
				case PERF_RECORD_SAMPLE: {
					clock_gettime(CLOCK_MONOTONIC, &cur_time);
					cur_record_count += 1;

					ps = (struct perf_sample*)ph;
					assert(ps != NULL);
					address = ps->addr >> target_page_size;
					data.addr = address;
					printf("addr: 0x%llx\n", data.addr);

					//fprintf(fp, "addr: 0x%llx\n", ps->addr); 
					//fprintf(fp, "addr: 0x%llx\n", address); 
					//printf("addr: 0x%llx\n", ps->addr);

					// update footprint
					bool bool_result = footprint_bf.query(data.str);
					printf("footprint: %d\n", bool_result);	
					printf("cur_record_count: %ld\n", cur_record_count);
					if (!footprint_bf.query(data.str)) {
						footprint_page_count += 1;	
						footprint_bf.insert(data.str);
					}

					// insert to CM Sketch
					//data.timestamp += 1; // uset memory access
					cur_second_phase = cur_time.tv_sec;
					cur_nano_stamp = cur_time.tv_sec * (int)1e9 + cur_time.tv_nsec;
					data.timestamp = cur_nano_stamp - first_nano_step; // use real time
					fre_sketch.insert(data);
					//printf("last: %lu cur: %lu\n", last_second_phase, cur_second_phase);

					// update hot page
					data_count = fre_sketch.query(data);
					printf("addr: 0x%llx data_cnt: %d\n", data.addr, data_count);
					printf("data_count: %d, last_record_count: %ld, rate: %lf\n", 
						data_count, last_record_count, data_count/ (double)last_record_count);
					if (((double)data_count / last_record_count) >  hot_threshold_rate && !hot_bf.query(data.str)) {
					//if (data_count > hot_threshold && !hot_bf.query(data.str)) {
						hot_page_count += 1;
						hot_bf.insert(data.str);
						printf("hot page addr: 0x%llx\n", address);
					}

					// print info and reset status
					//if (count >= QUERY_PERIOD) {
					if (cur_second_phase > last_second_phase) {
						last_second_phase = cur_second_phase;
						//assert(count == QUERY_PERIOD);
						printf("hot: %d footprint: %d (KB)\n",
							hot_page_count << target_page_size >> 10,
							footprint_page_count << target_page_size >> 10);
						printf("before: cur_record_count: %ld, last_record_cound: %ld\n", cur_record_count, last_record_count);
						last_record_count = cur_record_count;
						cur_record_count = 1;
						printf("after: cur_record_count: %ld, last_record_cound: %ld\n", cur_record_count, last_record_count);
						hot_page_count = 0;
						footprint_page_count = 0;

						footprint_bf.clear();
						hot_bf.clear();
					}

					break;
				}
				case PERF_RECORD_THROTTLE: {
				case PERF_RECORD_UNTHROTTLE: {
					//dprintf(STDERR_FILENO, "throttle or unthrottle\n");
					break;
				}
				}
				default: {
					dprintf(STDERR_FILENO, "error\n");
				}
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
	
	// open file for record result
	//fp = fopen("./result", "w+");
	
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
