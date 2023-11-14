#ifndef HLL_H
#define HLL_H

#include "sketch.h"

typedef uint8_t Value;

template<typename Key>
struct HLL_Entry{
    uint16_t pos;
    Value value;

    HLL_Entry(uint16_t _pos = 0, Value _value = 0):
            pos(_pos), value(_value){};
};

template<typename Key, uint32_t LENGTH>
class Child_HLL : public Sketch<Key>{
public:

    Value sketch[LENGTH];

    Child_HLL(){
        memset(sketch, 0, sizeof(Value) * LENGTH);
    }

    ~Child_HLL(){}

    void insert_one(const Key& packet){}

    HashMap query_all(){
        HashMap ret;
        return ret;
    }
};

template<typename Key, uint32_t LENGTH>
class HLL : public Sketch<Key>{
public:

    Value sketch[LENGTH];

    HLL(){
        memset(sketch, 0, sizeof(Value) * LENGTH);
    }

    ~HLL(){}

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
            sum += (1.0 / (1 << sketch[i]));
        }
        ret[0] = 0.7213 * LENGTH / (LENGTH + 1.079) * LENGTH / sum * LENGTH;
        return ret;
    }

    void Merge(HLL* temp){
        for(uint32_t i = 0;i < LENGTH;++i){
            sketch[i] = MAX(sketch[i], temp->sketch[i]);
        }
        delete temp;
    }

    void Merge(const HLL_Entry<Key>& temp){
        const uint16_t& pos = temp.pos;
        sketch[pos] = MAX(sketch[pos], temp.value);
    }
};

#endif