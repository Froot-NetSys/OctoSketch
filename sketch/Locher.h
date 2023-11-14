#ifndef LOCHER_H
#define LOCHER_H

#include "../common/shll.h"

#include "sketch.h"

struct Locher_Entry{
    uint32_t src;
    uint16_t hashPos;
    uint16_t pos;
    uint32_t value;

    Locher_Entry(uint32_t _src = 0, uint32_t _hashPos = 0, uint32_t _pos = 0, uint32_t _value = 0):
            src(_src), hashPos(_hashPos), pos(_pos), value(_value){};
};

template<uint32_t HASH_NUM, uint32_t LENGTH>
class Child_Locher : public Sketch<Packet>{
public:

    SHLL sketch[HASH_NUM][LENGTH];

    Child_Locher(){
        memset(sketch, 0, sizeof(SHLL) * HASH_NUM * LENGTH);
    }

    ~Child_Locher(){}

    void insert_one(const Packet& packet){}

    HashMap query_all(){
        HashMap ret;
        return ret;
    }
};

template<uint32_t HASH_NUM, uint32_t LENGTH, uint32_t HEAP_SIZE>
class Locher : public Sketch<Packet>{
public:
    typedef Heap<uint64_t, int32_t> myHeap;

    SHLL sketch[HASH_NUM][LENGTH];
    double distinct[HASH_NUM] = {0};
    myHeap* heap;

    Locher(){
        memset(sketch, 0, sizeof(SHLL) * HASH_NUM * LENGTH);
        heap = new myHeap(HEAP_SIZE);
    }

    ~Locher(){
        delete heap;
    }

    void insert_one(const Packet& packet){
        double estimation[HASH_NUM] = {0};
        for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
            uint32_t pos = hash(packet.src, hashPos) % LENGTH;
            distinct[hashPos] -= sketch[hashPos][pos].Query();
            sketch[hashPos][pos].Insert(packet.dst, hashPos);
            double est = sketch[hashPos][pos].Query();
            estimation[hashPos] = est - distinct[hashPos] / (LENGTH - 1);
            distinct[hashPos] += est;
        }
        heap->Insert(packet.src, MEDIAN3(estimation));
    }

    HashMap query_all(){
        return heap->AllQuery();
    }

    void Merge(Locher* temp){
        for(uint32_t i = 0;i < HASH_NUM;++i){
            distinct[i] = 0;
            for(uint32_t j = 0;j < LENGTH;++j){
                sketch[i][j].Merge(temp->sketch[i][j]);
                distinct[i] += sketch[i][j].Query();
            }
        }

        myHeap* check[2] = {heap, temp->heap};

        for(auto p : check) {
            for (uint32_t i = 0; i < p->mp->size(); ++i) {
                double estimation[HASH_NUM] = {0};
                for (uint32_t hashPos = 0; hashPos < HASH_NUM; ++hashPos) {
                    uint32_t pos = hash((uint32_t)p->heap[i].key, hashPos) % LENGTH;
                    double est = sketch[hashPos][pos].Query();
                    estimation[hashPos] = est - (distinct[hashPos] - est) / (LENGTH - 1);
                }
                heap->Insert(p->heap[i].key, MEDIAN3(estimation));
            }
        }

        delete temp;
    }

    void Merge(const Locher_Entry& temp){
        const uint16_t& hashPos = temp.hashPos;
        const uint16_t& pos = temp.pos;

        double before = sketch[hashPos][pos].Query();

        uint32_t inbucket_index = (temp.value & 0x1);
        uint32_t bucket_index = ((temp.value >> 1) & 0x7);
        uint8_t rank = MIN(15, __builtin_clz(temp.value) + 1);
        bool modify = false;

        Buckets& tempBucket = sketch[hashPos][pos].buckets[bucket_index];
        switch(inbucket_index){
            case 0:
                if(tempBucket.counter0 < rank){
                    tempBucket.counter0 = rank;
                    modify = true;
                    break;
                }
            case 1:
                if(tempBucket.counter1 < rank){
                    tempBucket.counter1 = rank;
                    modify = true;
                    break;
                }
        }

        if(modify){
            double after = sketch[hashPos][pos].Query();
            distinct[hashPos] += (after - before);
            after = after - (distinct[hashPos] - after) / (LENGTH - 1);

            if(after > heap->min()){
                double estimation[HASH_NUM] = {0};

                for(uint32_t tempHash = 0;tempHash < HASH_NUM;++tempHash){
                    uint32_t tempPos = hash(temp.src, tempHash) % LENGTH;
                    double est = sketch[tempHash][tempPos].Query();
                    estimation[tempHash] = est - (distinct[tempHash] - est) / (LENGTH - 1);
                }
                heap->Insert(temp.src, MEDIAN3(estimation));
            }
        }
    }
};

#endif