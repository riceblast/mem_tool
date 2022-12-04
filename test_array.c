#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>

#define size 100
int array[size];
int i, j;
long long unsigned k;
int tmp;

void sigint_hangler(int sig) 
{
	printf("\ntotal write: %lld", k);
	exit (0);
}

int main()
{
	if (signal(SIGINT, sigint_hangler) == SIG_ERR)
		printf("install signal handler error\n");

	int stack_addr_1 = 0x293;

	int *malloc_addr = (int *)malloc(sizeof(int));
	double stack_addr_2 = 0x578;

	int *mmap_addr = (int *)mmap(NULL, 4 * 1024, PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);	

	if ((long long int)mmap_addr == -1) {
		perror("mmap 出错");
		assert(0);
	}

	    printf("&array: 0x%llx, start: 0x%llx, end: 0x%llx\n", 
				(unsigned long long int)(&array), 
				(unsigned long long int)array, 
				(unsigned long long int)(array + size));
		printf("i: 0x%llx, j: 0x%llx, k: 0x%llx, tmp: 0x%llx\n", 
				(unsigned long long int)&i,
			   	(unsigned long long int)&j,
			   	(unsigned long long int)&k, (unsigned long long int)&tmp);
		printf("stack_addr_1: 0x%llx, stack_addr_2: 0x%llx\n", 
				(unsigned long long int)&stack_addr_1, 
				(unsigned long long int)&stack_addr_2);
		printf("malloc: 0x%llx, &malloc: 0x%llx, \nmmap: 0x%llx, &mmap: 0x%llx\n", 
				(unsigned long long int)malloc_addr, 
				(unsigned long long int)&malloc_addr, 
				(unsigned long long int)mmap_addr,
				(unsigned long long int)&mmap_addr);

	while(1) {
	    for (i = 0; i < size; i++) {
			//tmp = array[i];	
			//tmp = stack_addr_1;
			//tmp = (int)stack_addr_2;
			//tmp = *malloc_addr;
			//tmp = *mmap_addr;
	    	array[i] = 0x234;
			stack_addr_1 = 0x09;
			stack_addr_2 = 0.429;
			*malloc_addr = 0x873;
			*mmap_addr = 0x02942;
			k+= 1;
	    }	
	}
}
