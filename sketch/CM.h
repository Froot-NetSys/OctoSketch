#ifndef CM_H
#define CM_H

#include "sketch.h"

typedef uint8_t Value;

template<typename Key>
struct CM_Entry{
    Key key;
    uint16_t hashPos;
    uint16_t pos;
    Value value;

    CM_Entry(Key _key = 0, uint16_t _hashPos = 0, uint16_t _pos = 0, Value _value = 0):
            key(_key), hashPos(_hashPos), pos(_pos), value(_value){};
};

template<typename Key, uint32_t _HASH_NUM, uint32_t _LENGTH>
class Child_CM : public Sketch<Key>{
public:

    Value sketch[_HASH_NUM][_LENGTH];

    Child_CM(){
        memset(sketch, 0, sizeof(Value) * _HASH_NUM * _LENGTH);
    }

    ~Child_CM(){}

    void insert_one(const Key& packet){}

    HashMap query_all(){
        HashMap ret;
        return ret;
    }
};

template<typename Key, uint32_t _HASH_NUM, uint32_t _LENGTH, uint32_t _HEAP_SIZE>
class CM : public Sketch<Key>{
public:
    typedef Heap<Key, int32_t> myHeap;

    int32_t sketch[_HASH_NUM][_LENGTH];
    myHeap* heap;

    CM(){
        memset(sketch, 0, sizeof(int32_t) * _HASH_NUM * _LENGTH);
        heap = new myHeap(_HEAP_SIZE);
    }

    ~CM(){
        delete heap;
    }

    void insert_one(const Key& packet){
        int32_t minimum = 0x7fffffff;

        for(uint32_t hashPos = 0;hashPos < _HASH_NUM;++hashPos){
            uint32_t pos = hash(packet, hashPos) % _LENGTH;
            sketch[hashPos][pos] += 1;
            minimum = MIN(minimum, sketch[hashPos][pos]);
        }

        heap->Insert(packet, minimum);
    }

    HashMap query_all(){
        return heap->AllQuery();
    }

    void Merge(CM* temp){
        for(uint32_t i = 0;i < _HASH_NUM;++i){
            for(uint32_t j = 0;j < _LENGTH;++j){
                sketch[i][j] += temp->sketch[i][j];
            }
        }
        myHeap* check[2] = {heap, temp->heap};

        for(auto p : check){
            for(uint32_t i = 0;i < p->mp->size();++i){
                int32_t minimum = 0x7fffffff;
                for(uint32_t hashPos = 0;hashPos < _HASH_NUM;++hashPos){
                    uint32_t pos = hash(p->heap[i].key, hashPos) % _LENGTH;
                    minimum = MIN(minimum, sketch[hashPos][pos]);
                }
                heap->Insert(p->heap[i].key, minimum);
            }
        }

        delete temp;
    }

    void Merge(const CM_Entry<Key>& temp){
        const uint16_t& hashPos = temp.hashPos;
        const uint16_t& pos = temp.pos;

        sketch[hashPos][pos] += temp.value;

        if(sketch[hashPos][pos] > heap->min()){
            int32_t minimum = sketch[hashPos][pos];
            for(uint32_t tempHash = 0;tempHash < _HASH_NUM;++tempHash){
                uint32_t tempPos = hash(temp.key, tempHash) % _LENGTH;
                minimum = MIN(minimum, sketch[tempHash][tempPos]);
            }
            heap->Insert(temp.key, minimum);
        }
    }
};

#endif