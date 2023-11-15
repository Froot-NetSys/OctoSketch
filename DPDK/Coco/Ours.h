#ifndef COCO_OURS_H
#define COCO_OURS_H

#include "config.h"
#include "../template/Ours.h"

class Coco_Ours : public Ours<uint64_t, Coco_Entry<uint64_t>>{
public:

    typedef ReaderWriterQueue<Coco_Entry<uint64_t>> myQueue;

    static weak_atomic<int32_t> PROMASK;

    Sketch<uint64_t>* initialize_parent(){
        return new MyCoco();
    }

    Sketch<uint64_t>* initialize_child(){
       return new MyChild_Coco();
    }

    void insert_child(Sketch<uint64_t>* p, myQueue& q, const uint64_t& packet){
        auto keys = ((MyChild_Coco*)p)->keys;
        auto counters = ((MyChild_Coco*)p)->counters;

        uint32_t choice;
        uint16_t pos[2];

        *((uint32_t*)pos) = hash(packet, 0);

        if(keys[0][pos[0]] == packet){
            counters[0][pos[0]] += 1;

            if(counters[0][pos[0]] >= PROMASK){
                q.enqueue(Coco_Entry<uint64_t>(packet, pos[0], pos[1], counters[0][pos[0]]));
                counters[0][pos[0]] = 0;
            }

            return;
        }

        if(keys[1][pos[1]] == packet){
            counters[1][pos[1]] += 1;

            if(counters[1][pos[1]] >= PROMASK){
                q.enqueue(Coco_Entry<uint64_t>(packet, pos[0], pos[1], counters[1][pos[1]]));
                counters[1][pos[1]] = 0;
            }

            return;
        }

        choice = (counters[0][pos[0]] > counters[1][pos[1]]);
        counters[choice][pos[choice]] += 1;
        if(((MyChild_Coco*)p)->rng() % counters[choice][pos[choice]] == 0){
            keys[choice][pos[choice]] = packet;

            if(counters[choice][pos[choice]] >= PROMASK){
                q.enqueue(Coco_Entry<uint64_t>(packet, pos[0], pos[1], counters[choice][pos[choice]]));
                counters[choice][pos[choice]] = 0;
            }
        }
        else{
            if(counters[choice][pos[choice]] >= PROMASK){
                q.enqueue(Coco_Entry<uint64_t>(keys[choice][pos[choice]], pos[0], pos[1], counters[choice][pos[choice]], false));
                counters[choice][pos[choice]] = 0;
            }
        }
    }

    void merge(Sketch<uint64_t>* p, Coco_Entry<uint64_t> temp){
        ((MyCoco*)p)->Merge(temp);
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