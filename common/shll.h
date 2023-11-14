#ifndef SHLL_H
#define SHLL_H

#include "util.h"
#include "hash.h"
#include <math.h>

struct Buckets{
    uint8_t counter0 : 4;
    uint8_t counter1 : 4;

    void insert(uint32_t index, uint8_t rank){
        switch(index){
            case 0:
                if(counter0 < rank){
                    counter0 = rank;
                }
                return;
            case 1:
                if(counter1 < rank){
                    counter1 = rank;
                }
                return;
        }
    }

    double query(){
        return 1.0 / (1 << counter0) + 1.0 / (1 << counter1);
    }

    void merge(Buckets& other){
        counter0 = MAX(counter0, other.counter0);
        counter1 = MAX(counter1, other.counter1);
    }
};

struct SHLL{

    template<typename Key>
    void Insert(Key key, uint32_t seed = 0){
        uint32_t temp = hash(key, seed);
        uint32_t inbucket_index = (temp & 0x1);
        uint32_t bucket_index = ((temp >> 1) & 0x7);
        uint8_t rank = MIN(15, __builtin_clz(temp) + 1);

        buckets[bucket_index].insert(inbucket_index, rank);
    }

    double Query(){
        double sum = 0;
        for(uint32_t i = 0;i < 8;++i){
            sum += buckets[i].query();
        }
        return 0.673 * 256 / sum;
    }

    void Merge(SHLL& other){
        for(uint32_t i = 0;i < 8;++i){
            buckets[i].merge(other.buckets[i]);
        }
    }

    Buckets buckets[8];
};

#endif
