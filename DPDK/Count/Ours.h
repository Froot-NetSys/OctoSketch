#ifndef COUNT_OURS_H
#define COUNT_OURS_H

#include "config.h"
#include "../template/Ours.h"

class Count_Ours : public Ours<uint64_t, Count_Entry<uint64_t>>{
public:

    typedef ReaderWriterQueue<Count_Entry<uint64_t>> myQueue;

    static weak_atomic<int32_t> PROMASK;

    Sketch<uint64_t>* initialize_parent(){
        return new MyCount();
    }

    Sketch<uint64_t>* initialize_child(){
       return new MyChild_Count();
    }

    void insert_child(Sketch<uint64_t>* p, myQueue& q, const uint64_t& packet){
        auto sketch = ((MyChild_Count*)p)->sketch;
        uint32_t pos[HASH_NUM];
        int32_t incre[HASH_NUM];

        for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
            uint32_t hashNum = hash(packet, hashPos);
            pos[hashPos] = (hashNum >> 1) % LENGTH;
            incre[hashPos] = increment[hashNum & 1];
        }

        for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
            sketch[hashPos][pos[hashPos]] += incre[hashPos];
            if(sketch[hashPos][pos[hashPos]] * incre[hashPos] >= PROMASK){
                q.enqueue(Count_Entry<uint64_t>(packet, hashPos, pos[hashPos], sketch[hashPos][pos[hashPos]]));
                sketch[hashPos][pos[hashPos]] = 0;
            }
        }
    }

    void merge(Sketch<uint64_t>* p, Count_Entry<uint64_t> temp){
        ((MyCount*)p)->Merge(temp);
    }

    int32_t old_length = 0;
    uint64_t old_time = -1;
    int32_t new_thres, tmp_mask;

    void modify_threshold(){
        int32_t new_length = 0;
        for(uint32_t j = 0;j < NUM_RX_QUEUE;++j){
            new_length += que[j].size_approx();
        }

        int32_t gap = new_length - old_length;
        int32_t tmp_length = new_length + gap;
        new_thres = tmp_mask = PROMASK;

        if(tmp_length < LOWER_LENGTH && gap < 32){
            new_thres = tmp_mask - 1;
            new_thres = MIN(0x3f, new_thres);
            new_thres = MAX(0x7, new_thres);
            PROMASK = new_thres;
        }
        else if(tmp_length > UPPER_LENGTH && gap > -32){
            new_thres = tmp_mask + 1;
            new_thres = MIN(0x3f, new_thres);
            new_thres = MAX(0x7, new_thres);
            PROMASK = new_thres;
        }
        
        old_length = new_length;
    }
};

#endif