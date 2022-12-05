#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>

#define PERF_PAGES (1 + (1 << 16))
#define SAMPLE_PERIOD 100
#define PEBS_NPROCS 1

int pid = -1;
int cpuid = -1;
int pfd[PEBS_NPROCS];

static struct perf_event_mmap_page *perf_page[PEBS_NPROCS];
struct perf_sample {
  struct perf_event_header header;
  __u64 ip;
  __u32 pid, tid;    /* if PERF_SAMPLE_TID */
  __u64 addr;        /* if PERF_SAMPLE_ADDR */
  __u64 weight;      /* if PERF_SAMPLE_WEIGHT */
  /* __u64 data_src;    /\* if PERF_SAMPLE_DATA_SRC *\/ */
};

void *print_message_function(void *ptr);

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                   group_fd, flags);
    return ret;
}

static struct perf_event_mmap_page* perf_setup(__u64 config, __u64 config1, __u64 cpu, __u64 type)
{
  struct perf_event_attr attr;

  memset(&attr, 0, sizeof(struct perf_event_attr));

  attr.type = PERF_TYPE_RAW;
  attr.size = sizeof(struct perf_event_attr);

  attr.config = config;
  attr.config1 = config1;
  attr.sample_period = SAMPLE_PERIOD;

  attr.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_WEIGHT | PERF_SAMPLE_ADDR;
  attr.disabled = 0;
  //attr.inherit = 1;
  attr.exclude_kernel = 1;
  attr.exclude_hv = 1;
  attr.exclude_callchain_kernel = 1;
  attr.exclude_callchain_user = 1;
  attr.precise_ip = 1;

  pfd[cpu] = perf_event_open(&attr, pid, cpuid, -1, 0); 
  if(pfd[cpu] == -1) {
    perror("perf_event_open");
  }
  assert(pfd[cpu] != -1);

  size_t mmap_size = sysconf(_SC_PAGESIZE) * PERF_PAGES;
  /* printf("mmap_size = %zu\n", mmap_size); */
  struct perf_event_mmap_page *p = (struct perf_event_mmap_page *)mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, pfd[cpu], 0); 
  if(p == MAP_FAILED) {
    perror("mmap");
  }
  assert(p != MAP_FAILED);

  return p;
}

void *pebs_scan_thread(void *)
{
	int count = 0;
	for (;;) {
		for (int i = 0; i < PEBS_NPROCS; i++) {
		    struct perf_event_mmap_page *p = perf_page[i];
			// 转到数据区
		    char *pbuf = (char *)p + p->data_offset;

		    __sync_synchronize();

		    if (p->data_head == p->data_tail) {
		    	continue;
		    }

			while (p->data_tail != p->data_head) {
		        struct perf_event_header *ph = (struct perf_event_header *)(pbuf + (p->data_tail % p->data_size));
		        struct perf_sample* ps;

		        switch(ph->type) {
		        case PERF_RECORD_SAMPLE:
			    	count += 1;
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
}

int main(int argc, char *argv[])
{
	pthread_t scan_thread;

	if (argc != 3) {
		dprintf(STDERR_FILENO, "usage: sudo ./perf_test pid cpuid\n");
		exit(0);
	}
	pid = strtol(argv[1], (char**)NULL, 10);
	cpuid = strtol(argv[2], (char**)NULL, 10);
	printf("pid: %d\n", pid);
	printf("cpuid: %d\n", cpuid);

	for (int i = 0; i < PEBS_NPROCS; i++)	
		perf_page[i] = perf_setup(0x82d0, 0, i, -1); // evnt=MEM_INST_RETIRED.ALL_STORES
		//perf_page[i] = perf_setup(0x81d0, 0, i, -1); // evnt=MEM_INST_RETIRED.ALL_LOADS

	int r = pthread_create(&scan_thread, NULL, pebs_scan_thread, NULL);
	assert(r == 0);
	
	while (1) {}
}
