#ifndef ELASTIC_OURS_H
#define ELASTIC_OURS_H

#include "config.h"
#include "template/Ours.h"

template<typename Key, uint32_t thread_num>
class Elastic_Ours : public Ours<Key, Elastic_Entry<Key>, thread_num>{
public:

    typedef ReaderWriterQueue<Elastic_Entry<Key>> myQueue;

    Sketch<Key>* initialize_parent(){
        return new MyElastic<Key>();
    }

    Sketch<Key>* initialize_child(){
       return new MyChild_Elastic<Key>();
    }

    void insert_child(Sketch<Key>* p, myQueue& q, const Key& packet){
        auto sketch = ((MyChild_Elastic<Key>*)p)->sketch;
        auto buckets = ((MyChild_Elastic<Key>*)p)->buckets;

        uint32_t pos = hash(packet) % BUCKET_LENGTH, minPos = 0;
        int32_t minVal = 0x7fffffff;

        for (uint32_t j = 0; j < COUNTER_PER_BUCKET; j++){
            if(buckets[pos].ID[j] == packet) {
                buckets[pos].count[j] += 1;
                if(buckets[pos].count[j] >= PROMASK){
                    q.enqueue(Elastic_Entry<Key>(buckets[pos].ID[j], pos, buckets[pos].count[j], true));
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
                    q.enqueue(Elastic_Entry<Key>(buckets[pos].ID[minPos], position, sketch[position], false));
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
                q.enqueue(Elastic_Entry<Key>(packet, position, sketch[position], false));
                sketch[position] = 0;
            }
        }
    }

    void merge(Sketch<Key>* p, Elastic_Entry<Key> temp){
        ((MyElastic<Key>*)p)->Merge(temp);
    }
};


#endif
