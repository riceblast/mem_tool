#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

// 总共4G
const int array_num = 10;
//const int array_ele_cnt = 100 * 1024 * 1024; // 400M  
const int array_ele_cnt = 100 * 1000 * 1000; // 4 * 100M  

#define hot_size  3
#define cold_size  7
int hot_array_idx[hot_size] = {3, 6, 9}; // 保存热数据的数组下标
int cold_array_idx[cold_size] = {0, 1, 2, 4, 5,  7, 8, }; // 保存冷数据的数组下标

//#define hot_size  2
//#define cold_size  8
//int hot_array_idx[hot_size] = {3, 6}; // 保存热数据的数组下标
//int cold_array_idx[cold_size] = {0, 1, 2, 4, 5, 7, 8, 9}; // 保存冷数据的数组下标

//#define hot_size  1
//#define cold_size  9
//int hot_array_idx[hot_size] = {3}; // 保存热数据的数组下标
//int cold_array_idx[cold_size] = {0, 1, 2, 4, 5, 6, 7, 8, 9}; // 保存冷数据的数组下标

int main() {
	int **all_array = (int **)malloc(array_num * sizeof(int *));
	for (int i = 0; i < array_num; i++) {
		all_array[i] = (int *)mmap(NULL, 4 * array_ele_cnt, PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0); 

		if ((long long int)all_array[i] == -1) {
			perror("mmap 出错\n");
			exit(1);
		}
	}

	//while(1);

	int i_hot, k_hot;
	int i_cold, k_cold;
	int *cur_array;
	int step = 1000;
	int step_step = 4;
	for (int loop = 1; loop;) {
		i_hot = 0;
		k_hot = 0;
		i_cold = 0;
		k_cold = 0;
		for (int access_cnt = 0; access_cnt < array_num * array_ele_cnt; 
			access_cnt += step){
		    for (int idx = 1; idx <= 10; idx++) {
		    	// cold access
		    	//if (idx == 3 || idx == 6) {					
		    	//if (idx == 5) {					
				if (idx == 3 || idx == 6 || idx == 9) {
		    		cur_array = all_array[cold_array_idx[i_cold]];
		    		for (int i = 0; i < step; i += step_step) {
		    			cur_array[k_cold + i] = 204;
		    		}

		    		k_cold += step;
		    		if (k_cold >= array_ele_cnt) {
		    			k_cold = 0;
		    			i_cold = (i_cold + 1) % cold_size;
		    		}
		    	} else {
		    		cur_array = all_array[hot_array_idx[i_hot]];
		    		for (int i = 0; i < step; i++) {
		    			cur_array[k_hot + i] = 204;
		    		}
		    		
		    		k_hot += step;
		    		if (k_hot >= array_ele_cnt) {
		    			k_hot = 0;
		    			i_hot = (i_hot + 1) % hot_size;
		    		}
		    		
		    	}
		    }		
		}
	}


}
