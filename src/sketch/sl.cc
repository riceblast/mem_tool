#include "sl.h"
#include <iostream>
using namespace std;

Base_SL::Base_SL(uint64_t period, int memory_size, int _hash_number, int _field_num) : 
Sketch() 
{
    hash_number=_hash_number;
    field_num=_field_num;
    total = memory_size/(sizeof(int)*field_num);
    step = (double)(total) * (field_num-1) / period;
    hash_length = total/hash_number;
    posi_idx = 0;
    field_idx = 0;
    step_count = 0;
    bucket = new Unit[total];
    for (int i = 0; i < total; i++)
    {
        bucket[i].count = new int[field_num];
        bucket[i].field_num = field_num;
        memset(bucket[i].count, 0, field_num * sizeof(int));
    }
}

Base_SL::~Base_SL(){
    for (int i = 0; i < total; i++)
    {
        delete []bucket[i].count;
        
    }
    delete []bucket;
}

void 
Base_SL::update(unsigned long long int time){
    unsigned long long int num = time * step;
    for (; step_count < num; ++step_count)
    {
        bucket[posi_idx].count[(field_idx + 1) % field_num] = 0;
        posi_idx = (posi_idx + 1) % total;
        if (posi_idx == 0)
        {
            field_idx = (field_idx + 1) % field_num;
        }
    }
}

void Base_SL::insert(Data data)
{
    update(data.timestamp);
}

void CM_SL::insert(Data data)
{
    Base_SL::insert(data);
    unsigned int position;
    for (int i = 0; i < hash_number; ++i)
    {
        position = Hash(data.str, i, DATA_LEN) % hash_length + i * hash_length;
        //cout<<position<<endl;
        bucket[position].count[(field_idx + (position < posi_idx)) % field_num] += 1;
    }
}
int CM_SL::query(Data data)
{
    unsigned int min_num = 0x7fffffff;

    for (int i = 0; i < hash_number; ++i)
        min_num = MIN(bucket[Hash(data.str, i, DATA_LEN) % hash_length + i * hash_length].Total(), min_num);

    return min_num;
}

void CU_SL::insert(Data data)
{
    Base_SL::insert(data);
    int k = posi_idx / hash_length;
    unsigned int position = Hash(data.str, k, DATA_LEN) % hash_length + k * hash_length;
    if (position < posi_idx)
    {
        k = (k + 1) % hash_number;
        position = Hash(data.str, k, DATA_LEN) % hash_length + k * hash_length;
    }

    unsigned int height = bucket[position].count[(field_idx + (position < posi_idx)) % field_num];
    bucket[position].count[(field_idx + (position < posi_idx)) % field_num] += 1;

    for (int i = (k + 1) % hash_number; i != k; i = (i + 1) % hash_number)
    {
        position = Hash(data.str, i, DATA_LEN) % hash_length + i * hash_length;
        if (bucket[position].count[(field_idx + (position < posi_idx)) % field_num] <= height)
        {
            height = bucket[position].count[(field_idx + (position < posi_idx)) % field_num];
            bucket[position].count[(field_idx + (position < posi_idx)) % field_num] += 1;
        }
    }
}

int CU_SL::query(Data data)
{
    unsigned int min_num = 0x7fffffff;

    for (int i = 0; i < hash_number; ++i)
        min_num = MIN(bucket[Hash(data.str, i, DATA_LEN) % hash_length + i * hash_length].Total(), min_num);

    return min_num;
}

void CO_SL::insert(Data data)
{
    Base_SL::insert(data);
    unsigned int position, countHash;
    for (int i = 0; i < hash_number; ++i)
    {
        position = Hash(data.str, i, DATA_LEN) % hash_length + i * hash_length;
        countHash = Hash(data.str, i+1, DATA_LEN) & 1;
        bucket[position].count[(field_idx + (position < posi_idx)) % field_num] +=
            Count_Hash[countHash];
    }
}
int CO_SL::query(Data data)
{
    int *n = new int[hash_number];
    memset(n, 0, hash_number * sizeof(int));

    for (int i = 0; i < hash_number; ++i)
    {
        unsigned int position = Hash(data.str, i, DATA_LEN) % hash_length + i * hash_length;
        unsigned int countHash = Hash(data.str, i+1, DATA_LEN) & 1;
        n[i] = bucket[position].Total() * Count_Hash[countHash];
    }

    std::sort(n, n + hash_number);

    return Mid(n);
}
int CO_SL::Mid(int *num)
{
    if (hash_number & 1)
    {
        return std::max(num[hash_number >> 1], 0);
    }
    else
    {
        return std::max(0, (num[hash_number >> 1] + num[(hash_number >> 1) - 1]) >> 1);
    }
}