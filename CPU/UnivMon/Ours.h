#ifndef UNIV_OURS_H
#define UNIV_OURS_H

#include "config.h"
#include "template/Ours.h"

template<typename Key, uint32_t thread_num>
class Univ_Ours : public Ours<Key, Univ_Entry<Key>, thread_num>{
public:

    typedef ReaderWriterQueue<Univ_Entry<Key>> myQueue;

    Sketch<Key>* initialize_parent(){
        return new MyUniv<Key>();
    }

    Sketch<Key>* initialize_child(){
        return new MyChild_Univ<Key>();
    }

    void insert_child(Sketch<Key>* p, myQueue& q, const Key& packet){
        auto sketch = ((MyChild_Univ<Key>*)p)->sketch;

        uint32_t polar = hash(packet, 199);
        uint32_t max_level = MIN(MAX_LEVEL - 1, __builtin_clz(polar));

        for(uint32_t level = 0; level <= max_level;++level){
            uint32_t pos[HASH_NUM];
            int32_t incre[HASH_NUM];

            for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
                uint32_t hashNum = hash(packet, level * HASH_NUM + hashPos);
                pos[hashPos] = (hashNum >> 1) % LENGTH;
                incre[hashPos] = increment[hashNum & 1];
            }

            for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
                sketch[level][hashPos][pos[hashPos]] += incre[hashPos];
                if(sketch[level][hashPos][pos[hashPos]] * incre[hashPos] >= PROMASK){
                    q.enqueue(Univ_Entry<Key>(packet, level, hashPos, pos[hashPos],
                        sketch[level][hashPos][pos[hashPos]]));
                    sketch[level][hashPos][pos[hashPos]] = 0;
                }
            }
        }
    }

    void merge(Sketch<Key>* p, Univ_Entry<Key> temp){
        ((MyUniv<Key>*)p)->Merge(temp);
    }

};

#endif
