#ifndef COCO_H
#define COCO_H

#include "sketch.h"

typedef uint8_t Value;

template<typename Key>
struct Coco_Entry{
    Key key;
    uint16_t pos[2];
    Value value;
    bool pos_valid;

    Coco_Entry(Key _key = 0, uint16_t _pos0 = 0, uint16_t _pos1 = 0,
            Value _value = 0, uint16_t _pos_valid = true):
            key(_key), value(_value), pos_valid(_pos_valid){
        pos[0] = _pos0;
        pos[1] = _pos1;
    };
};

template<typename Key, uint32_t HASH_NUM, uint32_t LENGTH>
class Child_Coco : public Sketch<Key>{
public:

    Key keys[HASH_NUM][LENGTH];
    Value counters[HASH_NUM][LENGTH];

    std::mt19937 rng;

    Child_Coco(){
        memset(keys, 0, sizeof(Key) * HASH_NUM * LENGTH);
        memset(counters, 0, sizeof(Value) * HASH_NUM * LENGTH);

        std::random_device rd;
        rng.seed(rd());
    }

    ~Child_Coco(){}

    void insert_one(const Key& packet){}

    HashMap query_all(){
        HashMap ret;
        return ret;
    }
};

template<typename Key, uint32_t HASH_NUM, uint32_t LENGTH>
class Coco : public Sketch<Key>{
public:

    Key keys[HASH_NUM][LENGTH];
    int32_t counters[HASH_NUM][LENGTH];

    std::mt19937 rng;

    Coco(){
        memset(keys, 0, sizeof(Key) * HASH_NUM * LENGTH);
        memset(counters, 0, sizeof(int32_t) * HASH_NUM * LENGTH);

        std::random_device rd;
        rng.seed(rd());
    }

    ~Coco(){}

    void insert_one(const Key& packet){
        uint32_t choice;
        uint16_t pos[2];

        *((uint32_t*)pos) = hash(packet, 0);

        if(keys[0][pos[0]] == packet){
            counters[0][pos[0]] += 1;
            return;
        }

        if(keys[1][pos[1]] == packet){
            counters[1][pos[1]] += 1;
            return;
        }

        choice = (counters[0][pos[0]] > counters[1][pos[1]]);
        counters[choice][pos[choice]] += 1;
        if(rng() % counters[choice][pos[choice]] == 0){
            keys[choice][pos[choice]] = packet;
        }
    }

    HashMap query_all(){
        HashMap ret;

        for(uint32_t i = 0;i < HASH_NUM;++i){
            for(uint32_t j = 0;j < LENGTH;++j){
                ret[keys[i][j]] = counters[i][j];
            }
        }

        return ret;
    }

    void Merge(Coco* temp){
        for(uint32_t i = 0;i < HASH_NUM;++i){
            for(uint32_t j = 0;j < LENGTH;++j){
                counters[i][j] += temp->counters[i][j];
                if(counters[i][j] == 0 || rng() % counters[i][j] < temp->counters[i][j]){
                    keys[i][j] = temp->keys[i][j];
                }
            }
        }

        delete temp;
    }

    void Merge(Coco_Entry<Key> temp){
        if(!temp.pos_valid){
            *((uint32_t*)temp.pos) = hash(temp.key, 0);
        }

        if(keys[0][temp.pos[0]] == temp.key){
            counters[0][temp.pos[0]] += temp.value;
            return;
        }

        if(keys[1][temp.pos[1]] == temp.key){
            counters[1][temp.pos[1]] += temp.value;
            return;
        }

        uint32_t choice = (counters[0][temp.pos[0]] > counters[1][temp.pos[1]]);
        counters[choice][temp.pos[choice]] += temp.value;
        if(rng() % counters[choice][temp.pos[choice]] < temp.value){
            keys[choice][temp.pos[choice]] = temp.key;
        }
    }

};

#endif