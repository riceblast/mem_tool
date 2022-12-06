#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include "event.h"

const int SAMPLE_PERIOD = 100;
const int BUFFER_PAGE_NUM = (1 + (1 << 16)); // buffer_size: 4K + 256M;

struct perf_sample {
  struct perf_event_header header;
  __u64 ip;
  __u32 pid, tid;    /* if PERF_SAMPLE_TID */
  __u64 addr;        /* if PERF_SAMPLE_ADDR */
  __u64 weight;      /* if PERF_SAMPLE_WEIGHT */
  /* __u64 data_src;    /\* if PERF_SAMPLE_DATA_SRC *\/ */
};


class PebsFrontEnd
{
	private:	
		struct perf_event_attr attr;
		struct perf_event_mmap_page *buffer; // 目前仅考虑单个程序仅使用一个cpu核
		int event;
		int sample_period;
		int page_num;
		int pid, cpuid;
		int pfd;

		struct perf_event_mmap_page *_perf_setup(__u64 config, __u64 config1);
		long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags);
		void gather_data(void);

	public:
		struct perf_event_mmap_page *get_buffer(void);
		void perf_setup(void); 
		void perf_start(void);
		PebsFrontEnd(void);	
		PebsFrontEnd(int event, int pid, int cpuid);
};
