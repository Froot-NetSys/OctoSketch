#ifndef COUNT_OURS_H
#define COUNT_OURS_H

#include "config.h"
#include "template/Ours.h"

template<typename Key, uint32_t thread_num>
class Count_Ours : public Ours<Key, Count_Entry<Key>, thread_num>{
public:

    typedef ReaderWriterQueue<Count_Entry<Key>> myQueue;

    Sketch<Key>* initialize_parent(){
        return new MyCount<Key>();
    }

    Sketch<Key>* initialize_child(){
        return new MyChild_Count<Key>();
    }

    void insert_child(Sketch<Key>* p, myQueue& q, const Key& packet){
        auto sketch = ((MyChild_Count<Key>*)p)->sketch;
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
                q.enqueue(Count_Entry<Key>(packet, hashPos, pos[hashPos], sketch[hashPos][pos[hashPos]]));
                sketch[hashPos][pos[hashPos]] = 0;
            }
        }
    }

    void merge(Sketch<Key>* p, Count_Entry<Key> temp){
        ((MyCount<Key>*)p)->Merge(temp);
    }

};

#endif
