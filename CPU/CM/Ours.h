#ifndef CM_OURS_H
#define CM_OURS_H

#include "config.h"
#include "template/Ours.h"

template<typename Key, uint32_t thread_num>
class CM_Ours : public Ours<Key, CM_Entry<Key>, thread_num>{
public:

    typedef ReaderWriterQueue<CM_Entry<Key>> myQueue;

    Sketch<Key>* initialize_parent(){
        return new MyCM<Key>();
    }

    Sketch<Key>* initialize_child(){
       return new MyChild_CM<Key>();
    }

    void insert_child(Sketch<Key>* p, myQueue& q, const Key& packet){
        auto sketch = ((MyChild_CM<Key>*)p)->sketch;
        uint32_t pos[HASH_NUM];

        for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
            pos[hashPos] = hash(packet, hashPos) % LENGTH;
        }

        for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
            sketch[hashPos][pos[hashPos]] += 1;
            if(sketch[hashPos][pos[hashPos]] >= PROMASK){
                q.enqueue(CM_Entry<Key>(packet, hashPos, pos[hashPos], sketch[hashPos][pos[hashPos]]));
                sketch[hashPos][pos[hashPos]] = 0;
            }
        }
    }

    void merge(Sketch<Key>* p, CM_Entry<Key> temp){
        ((MyCM<Key>*)p)->Merge(temp);
    }

};

#endif