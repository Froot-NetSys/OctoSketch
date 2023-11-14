#ifndef COCO_OURS_H
#define COCO_OURS_H

#include "config.h"
#include "template/Ours.h"

template<typename Key, uint32_t thread_num>
class Coco_Ours : public Ours<Key, Coco_Entry<Key>, thread_num>{
public:

    typedef ReaderWriterQueue<Coco_Entry<Key>> myQueue;
    
    Sketch<Key>* initialize_parent(){
        return new MyCoco<Key>();
    }

    Sketch<Key>* initialize_child(){
       return new MyChild_Coco<Key>();
    }
       
    void insert_child(Sketch<Key>* p, myQueue& q, const Key& packet){
        auto keys = ((MyChild_Coco<Key>*)p)->keys;
        auto counters = ((MyChild_Coco<Key>*)p)->counters;

        uint32_t choice;
        uint16_t pos[2];

        *((uint32_t*)pos) = hash(packet, 0);

        if(keys[0][pos[0]] == packet){
            counters[0][pos[0]] += 1;

            if(counters[0][pos[0]] >= PROMASK){
                q.enqueue(Coco_Entry<Key>(packet, pos[0], pos[1], counters[0][pos[0]]));
                counters[0][pos[0]] = 0;
            }

            return;
        }

        if(keys[1][pos[1]] == packet){
            counters[1][pos[1]] += 1;

            if(counters[1][pos[1]] >= PROMASK){
                q.enqueue(Coco_Entry<Key>(packet, pos[0], pos[1], counters[1][pos[1]]));
                counters[1][pos[1]] = 0;
            }

            return;
        }

        choice = (counters[0][pos[0]] > counters[1][pos[1]]);
        counters[choice][pos[choice]] += 1;
        if(((MyChild_Coco<Key>*)p)->rng() % counters[choice][pos[choice]] == 0){
            keys[choice][pos[choice]] = packet;

            if(counters[choice][pos[choice]] >= PROMASK){
                q.enqueue(Coco_Entry<Key>(packet, pos[0], pos[1], counters[choice][pos[choice]]));
                counters[choice][pos[choice]] = 0;
            }
        }
        else{
            if(counters[choice][pos[choice]] >= PROMASK){
                q.enqueue(Coco_Entry<Key>(keys[choice][pos[choice]], pos[0], pos[1], counters[choice][pos[choice]], false));
                counters[choice][pos[choice]] = 0;
            }
        }
    }

    void merge(Sketch<Key>* p, Coco_Entry<Key> temp){
        ((MyCoco<Key>*)p)->Merge(temp);
    }

};

#endif
