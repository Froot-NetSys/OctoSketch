#ifndef LL_H
#define LL_H

#include "sketch.h"

typedef uint8_t Value;

template<typename Key>
struct LL_Entry{
    uint16_t pos;
    Value value;

    LL_Entry(uint16_t _pos = 0, Value _value = 0):
            pos(_pos), value(_value){};
};

template<typename Key, uint32_t LENGTH>
class Child_LL : public Sketch<Key>{
public:

    Value sketch[LENGTH];

    Child_LL(){
        memset(sketch, 0, sizeof(Value) * LENGTH);
    }

    ~Child_LL(){}

    void insert_one(const Key& packet){}

    HashMap query_all(){
        HashMap ret;
        return ret;
    }
};

template<typename Key, uint32_t LENGTH>
class LL : public Sketch<Key>{
public:

    Value sketch[LENGTH];

    LL(){
        memset(sketch, 0, sizeof(Value) * LENGTH);
    }

    ~LL(){}

    void insert_one(const Key& packet){
        uint32_t pos = hash(packet) % LENGTH;
        uint32_t temp = hash(packet, 101);
        uint8_t rank = MIN(31, __builtin_clz(temp) + 1);

        if(sketch[pos] < rank)
            sketch[pos] = rank;
    }

    HashMap query_all(){
        HashMap ret;
        double sum = 0;
        for(uint32_t i = 0;i < LENGTH;++i){
            sum += sketch[i];
        }
        ret[0] = 0.39701 * LENGTH * pow(2, sum / LENGTH);
        return ret;
    }

    void Merge(LL* temp){
        for(uint32_t i = 0;i < LENGTH;++i){
            sketch[i] = MAX(sketch[i], temp->sketch[i]);
        }
        delete temp;
    }

    void Merge(const LL_Entry<Key>& temp){
        const uint16_t& pos = temp.pos;
        sketch[pos] = MAX(sketch[pos], temp.value);
    }
};

#endif