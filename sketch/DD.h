#ifndef DD_H
#define DD_H

#include "sketch.h"
#include <cmath>

typedef uint8_t Value;

template<typename Key>
struct DD_Entry{
    uint16_t pos;
    Value value;

    DD_Entry(uint16_t _pos = 0, Value _value = 0):
            pos(_pos), value(_value){};
};

template<typename Key, uint32_t LENGTH>
class Child_DD : public Sketch<Key> {
public:

    Value sketch[LENGTH];

    Child_DD(){
        memset(sketch, 0, sizeof(Value) * LENGTH);
    }

    ~Child_DD(){}

    void insert_one(const Key& packet){}

    HashMap query_all(){
        HashMap ret;
        return ret;
    }

};

#define GAMMA 1.001

template<typename Key, uint32_t LENGTH>
class DD : public Sketch<Key>{
public:

    int32_t sketch[LENGTH];

    DD(){
        memset(sketch, 0, sizeof(int32_t) * LENGTH);
    }

    ~DD(){}

    void insert_one(const Key& packet){
        uint32_t pos = log(packet) / log(GAMMA);
        if (pos >= LENGTH) {
            pos = LENGTH - 1;
        }
        sketch[pos] += 1; // if cannot fit, put in the last bucket
    }

    HashMap query_all(){
        HashMap ret;

        double sum = 0;
        for(uint32_t i = 0;i < LENGTH;++i){
            sum += sketch[i];
        }

        uint32_t index = 0;
        double preSum = 0;
        for(uint32_t i = 0; i < 1000; ++i){
            while(preSum <= i * 0.001 * sum){
                preSum += sketch[index];
                index += 1;
            }
            ret[i] = 2 * pow(GAMMA, index) / (1 + GAMMA);
        }

        return ret;
    }

    void Merge(DD* temp){
        for(uint32_t i = 0;i < LENGTH;++i){
            sketch[i] += temp->sketch[i];
        }
        delete temp;
    }

    void Merge(const DD_Entry<Key>& temp){
        const uint16_t& pos = temp.pos;
        sketch[pos] += temp.value;
    }
};

#endif