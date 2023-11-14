#ifndef LL_OURS_H
#define LL_OURS_H

#include "config.h"
#include "template/Ours.h"

template<typename Key, uint32_t thread_num>
class LL_Ours : public Ours<Key, LL_Entry<Key>, thread_num>{
public:

    typedef ReaderWriterQueue<LL_Entry<Key>> myQueue;

    Sketch<Key>* initialize_parent(){
        return new MyLL<Key>();
    }

    Sketch<Key>* initialize_child(){
       return new MyChild_LL<Key>();
    }

    void insert_child(Sketch<Key>* p, myQueue& q, const Key& packet){
        auto sketch = ((MyChild_LL<Key>*)p)->sketch;

        uint32_t pos = hash(packet) % LENGTH;
        uint32_t temp = hash(packet, 101);
        uint8_t rank = MIN(31, __builtin_clz(temp) + 1);

        if(sketch[pos] < rank){
            sketch[pos] = rank;
            if(rank > PROMASK){
                q.enqueue(LL_Entry<Key>(pos, sketch[pos]));
            }
        }
    }

    void merge(Sketch<Key>* p, LL_Entry<Key> temp){
        ((MyLL<Key>*)p)->Merge(temp);
    }

};

#endif
