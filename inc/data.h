#ifndef DATA_H
#define DATA_H

#include "definition.h"
#include "hash_class.h"

class Data{
public:  
    //int head;
    //uint64_t timestamp;
    uint64_t timestamp;
    union {
		unsigned char str[DATA_LEN];
		long long unsigned int addr;
	};

    Data& operator = (Data an);
};

bool operator < (Data bn, Data an);
bool operator == (Data bn, Data an);

class My_Hash{
public:
    size_t operator()(const Data dat) const{
        return RSHash(dat.str, DATA_LEN);
    }
};

#endif // DATA_H
