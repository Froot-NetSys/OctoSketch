#ifndef ELASTIC_OURS_H
#define ELASTIC_OURS_H

#include "config.h"
#include "../template/Ours.h"

class Elastic_Ours : public Ours<uint64_t, Elastic_Entry<uint64_t>>{
public:

    typedef ReaderWriterQueue<Elastic_Entry<uint64_t>> myQueue;

    static weak_atomic<int32_t> PROMASK;

    Sketch<uint64_t>* initialize_parent(){
        return new MyElastic();
    }

    Sketch<uint64_t>* initialize_child(){
       return new MyChild_Elastic();
    }

    void insert_child(Sketch<uint64_t>* p, myQueue& q, const uint64_t& packet){
        auto sketch = ((MyChild_Elastic*)p)->sketch;
        auto buckets = ((MyChild_Elastic*)p)->buckets;

        uint32_t pos = hash(packet) % BUCKET_LENGTH, minPos = 0;
        int32_t minVal = 0x7fffffff;

        for (uint32_t j = 0; j < COUNTER_PER_BUCKET; j++){
            if(buckets[pos].ID[j] == packet) {
                buckets[pos].count[j] += 1;
                if(buckets[pos].count[j] >= PROMASK){
                    q.enqueue(Elastic_Entry<uint64_t>(buckets[pos].ID[j], pos, buckets[pos].count[j], true));
                    buckets[pos].count[j] = 0;
                }
                return;
            }

            if(buckets[pos].count[j] < minVal){
                minPos = j;
                minVal = buckets[pos].count[j];
            }
        }

        if((buckets[pos].vote + 1) >= minVal * 8){
            buckets[pos].vote = 0;

            if(minVal != 0){
                uint32_t position = hash(buckets[pos].ID[minPos], 101) % SKETCH_LENGTH;
                sketch[position] = sketch[position] + buckets[pos].count[minPos];
                if(sketch[position] >= PROMASK){
                    q.enqueue(Elastic_Entry<uint64_t>(buckets[pos].ID[minPos], position, sketch[position], false));
                    sketch[position] = 0;
                }
            }

            buckets[pos].ID[minPos] = packet;
            buckets[pos].count[minPos] = 1;
        }
        else {
            buckets[pos].vote += 1;
            uint32_t position = hash(packet, 101) % SKETCH_LENGTH;
            sketch[position] = sketch[position] + 1;
            if(sketch[position] >= PROMASK){
                q.enqueue(Elastic_Entry<uint64_t>(packet, position, sketch[position], false));
                sketch[position] = 0;
            }
        }
    }

    void merge(Sketch<uint64_t>* p, Elastic_Entry<uint64_t> temp){
        ((MyElastic*)p)->Merge(temp);
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