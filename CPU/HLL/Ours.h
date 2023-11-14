#ifndef HLL_OURS_H
#define HLL_OURS_H

#include "config.h"
#include "template/Ours.h"

template<typename Key, uint32_t thread_num>
class HLL_Ours : public Ours<Key, HLL_Entry<Key>, thread_num>{
public:

    typedef ReaderWriterQueue<HLL_Entry<Key>> myQueue;

    Sketch<Key>* initialize_parent(){
        return new MyHLL<Key>();
    }

    Sketch<Key>* initialize_child(){
       return new MyChild_HLL<Key>();
    }

    void insert_child(Sketch<Key>* p, myQueue& q, const Key& packet){
        auto sketch = ((MyChild_HLL<Key>*)p)->sketch;

        uint32_t pos = hash(packet) % LENGTH;
        uint32_t temp = hash(packet, 101);
        uint8_t rank = MIN(31, __builtin_clz(temp) + 1);

        if(sketch[pos] < rank){
            sketch[pos] = rank;
            if(rank > PROMASK){
                q.enqueue(HLL_Entry<Key>(pos, sketch[pos]));
            }
        }
    }

    void merge(Sketch<Key>* p, HLL_Entry<Key> temp){
        ((MyHLL<Key>*)p)->Merge(temp);
    }

};

#endif
