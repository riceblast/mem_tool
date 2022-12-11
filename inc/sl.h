#ifndef SL_H
#define SL_H

#include <iostream>
#include <string.h>
#include <algorithm>
#include "data.h"
#include "definition.h"
#include "hash_class.h"

// use memory access time to construct sliding window
//const int WINDOW_SIZE = 2 * 1e5; // 800M / 4K
//const int QUERY_PERIOD = 1 * 1e5; // WINDOW_SIZE * 1/2

// use time to  construct slidin window
// nano second
const int WINDOW_SIZE = (int)10 * 1e9;
const int QUERY_PERIOD = (int)10 * 1e9;

class Sketch{
public :
    Sketch(){};
    virtual void insert(Data) =0;
    virtual int query(Data) =0;
    //virtual ~Sketch()=0;
};

class Base_SL: public Sketch{
public:
    int hash_length;
    int hash_number;
    int field_num;
    int total;
    int posi_idx;
    int field_idx;
    double step;
    unsigned long long int step_count;
    struct Unit{
        int* count;
        int field_num;
        int Total(){
            int ret = 0;
            for(int i = 0;i < field_num;++i){
                ret += count[i];
            }
            return ret;
        }
    };
    Unit* bucket;
    Base_SL(uint64_t period, int memory_size, int hash_number, int field_num);

    virtual ~Base_SL();

    virtual void insert(Data);
    virtual int query(Data) =0;
    void update(unsigned long long int time);
};

class CM_SL: public Base_SL{
    public:
    CM_SL(uint64_t period, int memory_size, int hash_number, int field_num):
    Base_SL(period,memory_size,hash_number,field_num)
    {    }
    void insert(Data);
    int query(Data);

};

static int Count_Hash[2] = {-1, 1};
class CO_SL: public Base_SL{
    public:
    CO_SL(uint64_t period, int memory_size, int hash_number, int field_num):
    Base_SL(period,memory_size,hash_number,field_num)
    {    }
    void insert(Data);
    int query(Data);
    int Mid(int *num);
};

class CU_SL: public Base_SL{
    public:
    CU_SL(uint64_t period, int memory_size, int hash_number, int field_num):
    Base_SL(period,memory_size,hash_number,field_num)
    {    }
    void insert(Data);
    int query(Data);

};


#endif // CLOCK_H
