#ifndef LOCHER_OURS_H
#define LOCHER_OURS_H

#include "config.h"
#include "template/Ours.h"

template<uint32_t thread_num>
class Locher_Ours : public Ours<Packet, Locher_Entry, thread_num>{
public:

    typedef ReaderWriterQueue<Locher_Entry> myQueue;

    Sketch<Packet>* initialize_parent(){
        MyLocher* ret = new MyLocher();
        memset(ret->sketch, HARDTHRES, HASH_NUM * LENGTH * sizeof(SHLL));
        return ret;
    }

    Sketch<Packet>* initialize_child(){
        MyChild_Locher* ret = new MyChild_Locher();
        memset(ret->sketch, HARDTHRES, HASH_NUM * LENGTH * sizeof(SHLL));
        return ret;
    }

    void insert_child(Sketch<Packet>* p, myQueue& q, const Packet& packet){
        auto sketch = ((MyChild_Locher*)p)->sketch;
        
        uint32_t pos[HASH_NUM];
        uint32_t temp[HASH_NUM];

        for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
            pos[hashPos] = hash(packet.src, hashPos) % LENGTH;
            temp[hashPos] = hash(packet.dst, hashPos);
        }

        for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
            uint32_t bucket_index = ((temp[hashPos] >> 1) & 0x7);
            Buckets& tempBucket = sketch[hashPos][pos[hashPos]].buckets[bucket_index];
            
            uint32_t inbucket_index = (temp[hashPos] & 0x1);
            uint8_t rank = MIN(15, __builtin_clz(temp[hashPos]) + 1);

            switch(inbucket_index){
                case 0:
                    if(tempBucket.counter0 < rank){
                        tempBucket.counter0 = rank;
                        q.enqueue(Locher_Entry(packet.src, hashPos, pos[hashPos], temp[hashPos]));
                    }
                    break;
                case 1:
                    if(tempBucket.counter1 < rank){
                        tempBucket.counter1 = rank;
                        q.enqueue(Locher_Entry(packet.src, hashPos, pos[hashPos], temp[hashPos]));
                    }
                    break;
            }
        }
    }

    void merge(Sketch<Packet>* p, Locher_Entry temp){
        ((MyLocher*)p)->Merge(temp);
    }

};

#endif
