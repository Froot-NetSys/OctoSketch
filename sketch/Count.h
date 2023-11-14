#ifndef COUNT_H
#define COUNT_H

#include "sketch.h"

typedef int8_t Value;

constexpr static int32_t increment[2] = {1, -1};

template<typename Key>
struct Count_Entry{
    Key key;
    uint16_t hashPos;
    uint16_t pos;
    Value value;

    Count_Entry(Key _key = 0, uint16_t _hashPos = 0, uint16_t _pos = 0, Value _value = 0):
            key(_key), hashPos(_hashPos), pos(_pos), value(_value){};
};

template<typename Key, uint32_t HASH_NUM, uint32_t LENGTH>
class Child_Count : public Sketch<Key>{
public:

    Value sketch[HASH_NUM][LENGTH];

    Child_Count(){
        memset(sketch, 0, sizeof(Value) * HASH_NUM * LENGTH);
    }

    ~Child_Count(){}

    void insert_one(const Key& packet){}

    HashMap query_all(){
        HashMap ret;
        return ret;
    }
};

template<typename Key, uint32_t HASH_NUM, uint32_t LENGTH, uint32_t HEAP_SIZE>
class Count : public Sketch<Key>{
public:
    typedef Heap<Key, int32_t> myHeap;

    int32_t sketch[HASH_NUM][LENGTH];
    myHeap* heap;

    Count(){
        memset(sketch, 0, sizeof(int32_t) * HASH_NUM * LENGTH);
        heap = new myHeap(HEAP_SIZE);
    }

    ~Count(){
        delete heap;
    }

    void insert_one(const Key& packet){
        int32_t number[HASH_NUM] = {0};

        for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
            uint32_t hashNum = hash(packet, hashPos);
            uint32_t pos = (hashNum >> 1) % LENGTH;
            int32_t incre = increment[hashNum & 1];

            sketch[hashPos][pos] += incre;
            number[hashPos] = sketch[hashPos][pos] * incre;
        }
        
        heap->Insert(packet, MEDIAN3(number));
    }

    HashMap query_all(){
        return heap->AllQuery();
    }

    void Merge(Count* temp){
        for(uint32_t i = 0;i < HASH_NUM;++i){
            for(uint32_t j = 0;j < LENGTH;++j){
                sketch[i][j] += temp->sketch[i][j];
            }
        }

        myHeap* check[2] = {heap, temp->heap};

        for(auto p : check){
            for(uint32_t i = 0;i < p->mp->size();++i){
                int32_t count[HASH_NUM] = {0};
                for (uint32_t hashPos = 0; hashPos < HASH_NUM; ++hashPos) {
                    uint32_t hashNum = hash(p->heap[i].key, hashPos);
                    uint32_t pos = (hashNum >> 1) % LENGTH;
                    int32_t incre = increment[hashNum & 1];
                    count[hashPos] = sketch[hashPos][pos] * incre;
                }
                heap->Insert(p->heap[i].key, MEDIAN3(count));
            }
        }

        delete temp;
    }

    void Merge(const Count_Entry<Key>& temp){
        const uint16_t& hashPos = temp.hashPos;
        const uint16_t& pos = temp.pos;

        sketch[hashPos][pos] += temp.value;

        if(abs(sketch[hashPos][pos]) > heap->min()){
            int32_t count[HASH_NUM] = {0};

            for(uint32_t tempHash = 0;tempHash < HASH_NUM;++tempHash){
                uint32_t hashNum = hash(temp.key, tempHash);
                uint32_t tempPos = (hashNum >> 1) % LENGTH;
                int32_t incre = increment[hashNum & 1];

                count[tempHash] = sketch[tempHash][tempPos] * incre;
            }
            heap->Insert(temp.key, MEDIAN3(count));
        }
    }
};

#endif