#ifndef UNIVMON_H
#define UNIVMON_H

#include "sketch.h"

typedef int8_t Value;

constexpr static int32_t increment[2] = {1, -1};

template<typename Key>
struct Univ_Entry{
    Key key;
    uint16_t level;
    uint16_t hashPos;
    uint16_t pos;
    Value value;

    Univ_Entry(Key _key = 0, uint16_t _level = 0, uint16_t _hashPos = 0,
            uint16_t _pos = 0, Value _value = 0):
            key(_key), level(_level), hashPos(_hashPos), pos(_pos), value(_value){};
};

template<typename Key, uint32_t MAX_LEVEL, uint32_t HASH_NUM, uint32_t LENGTH>
class Child_UnivMon : public Sketch<Key>{
public:

    Value sketch[MAX_LEVEL][HASH_NUM][LENGTH];

    Child_UnivMon(){
        memset(sketch, 0, sizeof(Value) * MAX_LEVEL * HASH_NUM * LENGTH);
    }

    ~Child_UnivMon(){}

    void insert_one(const Key& packet){}

    HashMap query_all(){
        HashMap ret;
        return ret;
    }
};

template<typename Key, uint32_t MAX_LEVEL, uint32_t HASH_NUM, uint32_t LENGTH, uint32_t HEAP_SIZE>
class UnivMon : public Sketch<Key>{
public:
    typedef Heap<Key, int32_t> myHeap;

    int32_t sketch[MAX_LEVEL][HASH_NUM][LENGTH];
    myHeap* heap[MAX_LEVEL];

    UnivMon(){
        memset(sketch, 0, sizeof(int32_t) * MAX_LEVEL * HASH_NUM * LENGTH);
        for(uint32_t i = 0;i < MAX_LEVEL;++i){
           heap[i] = new myHeap(HEAP_SIZE);
        }
    }

    ~UnivMon(){
        for(uint32_t i = 0;i < MAX_LEVEL;++i){
            delete heap[i];
        }
    }

    void insert_one(const Key& packet){
        uint32_t polar = hash(packet, 199);
        uint32_t max_level = MIN(MAX_LEVEL - 1, __builtin_clz(polar));

        for(uint32_t level = 0; level <= max_level;++level){
            int32_t number[HASH_NUM] = {0};
            for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
                uint32_t hashNum = hash(packet, level * HASH_NUM + hashPos);
                uint32_t pos = (hashNum >> 1) % LENGTH;
                int32_t incre = increment[hashNum & 1];

                sketch[level][hashPos][pos] += incre;
                number[hashPos] = sketch[level][hashPos][pos] * incre;
            }
            heap[level]->Insert(packet, MEDIAN3(number));
        }
    }

    HashMap query_all(){
        HashMap ret;

        for(int32_t level = 0;level < MAX_LEVEL;++level){
            HashMap temp = heap[level]->AllQuery();
            for(auto it = temp.begin();it != temp.end();++it){
                if(ret.find(it->first) == ret.end()){
                    ret[it->first] = it->second;
                }
            }
        }

        return ret;
    }

    void Merge(UnivMon* temp){
        for(uint32_t level = 0;level < MAX_LEVEL;++level){
            for(uint32_t i = 0;i < HASH_NUM;++i){
                for(uint32_t j = 0;j < LENGTH;++j){
                    sketch[level][i][j] += temp->sketch[level][i][j];
                }
            }

            myHeap* check[2] = {heap[level], temp->heap[level]};

            for(auto p : check) {
                for(uint32_t i = 0;i < p->mp->size();++i){
                    int32_t number[HASH_NUM] = {0};
                    for(uint32_t hashPos = 0;hashPos < HASH_NUM;++hashPos){
                        uint32_t hashNum = hash(p->heap[i].key, level * HASH_NUM + hashPos);
                        uint32_t pos = (hashNum >> 1) % LENGTH;
                        int32_t incre = increment[hashNum & 1];
                        number[hashPos] = sketch[level][hashPos][pos] * incre;
                    }
                    heap[level]->Insert(p->heap[i].key, MEDIAN3(number));
                }
            }
        }

        delete temp;
    }

    void Merge(const Univ_Entry<Key>& temp){
        const uint16_t& level = temp.level;
        const uint16_t& hashPos = temp.hashPos;
        const uint16_t& pos = temp.pos;

        sketch[level][hashPos][pos] += temp.value;

        if(abs(sketch[level][hashPos][pos]) > heap[level]->min()){
            int32_t count[HASH_NUM] = {0};

            for(uint32_t tempHash = 0;tempHash < HASH_NUM;++tempHash){
                uint32_t hashNum = hash(temp.key, tempHash);
                uint32_t tempPos = (hashNum >> 1) % LENGTH;
                int32_t incre = increment[hashNum & 1];

                count[tempHash] = sketch[level][tempHash][tempPos] * incre;
            }
            heap[level]->Insert(temp.key, MEDIAN3(count));
        }
    }
};


#endif