#include "pebs_frontend.h"

PebsFrontEnd:: PebsFrontEnd(void)
{
	event = EMPTY_EVENT;
	buffer = NULL;
	page_num = 0;
	sample_period = 0;
	pid = 0;
	cpuid = 0;
	memset(&attr, 0, sizeof(struct perf_event_attr));
}

PebsFrontEnd::PebsFrontEnd(int event, int pid, int cpuid)
{
	this->event = event;
	this->pid = pid;
	this->cpuid = cpuid;
	page_num = BUFFER_PAGE_NUM;
	sample_period = SAMPLE_PERIOD;
	memset(&attr, 0, sizeof(struct perf_event_attr));
}

struct perf_event_mmap_page *PebsFrontEnd::get_buffer(void)
{
	return this->buffer;
}

long PebsFrontEnd::perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
		                int cpu, int group_fd, unsigned long flags)
{
	int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
			                   group_fd, flags);
	return ret;
}

struct perf_event_mmap_page *PebsFrontEnd::_perf_setup(__u64 config, __u64 config1)
{
  attr.type = PERF_TYPE_RAW;
  attr.size = sizeof(struct perf_event_attr);

  attr.config = config;
  attr.config1 = config1;
  attr.sample_period = sample_period;

  attr.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_WEIGHT | PERF_SAMPLE_ADDR;
  attr.disabled = 0;
  //attr.inherit = 1;
  attr.exclude_kernel = 1;
  attr.exclude_hv = 1;
  attr.exclude_callchain_kernel = 1;
  attr.exclude_callchain_user = 1;
  attr.precise_ip = 1;

	pfd = perf_event_open(&attr, pid, cpuid, -1, 0);
  if(pfd == -1) {
    perror("perf_event_open");
  }
  assert(pfd != -1);

  size_t mmap_size = sysconf(_SC_PAGESIZE) * page_num;
  /* printf("mmap_size = %zu\n", mmap_size); */
  struct perf_event_mmap_page *p = (struct perf_event_mmap_page *)mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, pfd, 0);
  if(p == MAP_FAILED) {
    perror("mmap");
  }
  assert(p != MAP_FAILED);

  return p;
}

void PebsFrontEnd::gather_data(void)
{
	int count = 0;
	for (;;) {
	    struct perf_event_mmap_page *p = buffer;
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

void PebsFrontEnd::perf_setup(void)
{
	this->buffer = _perf_setup(event, 0);	
}

void PebsFrontEnd::perf_start(void)
{
	//ioctl(pfd,PERF_EVENT_IOC_RESET,0);
    //ioctl(pfd,PERF_EVENT_IOC_ENABLE,0);
	gather_data();
}
