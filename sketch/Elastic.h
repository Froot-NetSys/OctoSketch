#ifndef ELASTIC_H
#define ELASTIC_H

#include "sketch.h"

typedef uint8_t Value;

template<typename Key>
struct Elastic_Entry{
    Key key;
    uint32_t pos;
    Value value;
    bool bucket;

    Elastic_Entry(Key _key = 0, uint32_t _pos = 0, Value _value = 0, bool _bucket = false):
            key(_key), pos(_pos), value(_value), bucket(_bucket){};
};

template<typename Key, uint32_t SKETCH_LENGTH, uint32_t BUCKET_LENGTH, uint32_t COUNTER_PER_BUCKET>
class Child_Elastic : public Sketch<Key>{
public:

    struct Bucket{
        int32_t vote;
        Key ID[COUNTER_PER_BUCKET];
        Value count[COUNTER_PER_BUCKET];
    };

    Value sketch[SKETCH_LENGTH];
    Bucket buckets[BUCKET_LENGTH];

    Child_Elastic(){
        memset(sketch, 0, sizeof(Value) * SKETCH_LENGTH);
        memset(buckets, 0, sizeof(Bucket) * BUCKET_LENGTH);
    }

    ~Child_Elastic(){}

    void insert_one(const Key& packet){}

    HashMap query_all(){
        HashMap ret;
        return ret;
    }
};

template<typename Key, uint32_t SKETCH_LENGTH, uint32_t BUCKET_LENGTH, uint32_t COUNTER_PER_BUCKET>
class Elastic : public Sketch<Key>{
public:
    
    struct Bucket{
        int32_t vote;
        Key ID[COUNTER_PER_BUCKET];
        int32_t count[COUNTER_PER_BUCKET];
    };

    int32_t sketch[SKETCH_LENGTH];
    Bucket buckets[BUCKET_LENGTH];

    Elastic(){
        memset(sketch, 0, sizeof(int32_t) * SKETCH_LENGTH);
        memset(buckets, 0, sizeof(Bucket) * BUCKET_LENGTH);
    }

    ~Elastic(){}

    void insert_one(const Key& packet){
        uint32_t pos = hash(packet) % BUCKET_LENGTH, minPos = 0;
        int32_t minVal = 0x7fffffff;

        for (uint32_t j = 0; j < COUNTER_PER_BUCKET; j++){
            if(buckets[pos].ID[j] == packet) {
                buckets[pos].count[j] += 1;
                return;
            }

            if(buckets[pos].count[j] == 0){
                buckets[pos].ID[j] = packet;
                buckets[pos].count[j] = 1;
                return;
            }

            if(buckets[pos].count[j] < minVal){
                minPos = j;
                minVal = buckets[pos].count[j];
            }
        }

        if((buckets[pos].vote + 1) >= minVal * 8){
            buckets[pos].vote = 0;

            uint32_t position = hash(buckets[pos].ID[minPos], 101) % SKETCH_LENGTH;
            sketch[position] = sketch[position] + buckets[pos].count[minPos];

            buckets[pos].ID[minPos] = packet;
            buckets[pos].count[minPos] = 1;
        }
        else {
            buckets[pos].vote += 1;
            uint32_t position = hash(packet, 101) % SKETCH_LENGTH;
            sketch[position] = sketch[position] + 1;
        }
    }

    HashMap query_all(){
        HashMap ret;

        for(uint32_t i = 0;i < BUCKET_LENGTH;++i){
            for(uint32_t j = 0;j < COUNTER_PER_BUCKET;++j){
                ret[buckets[i].ID[j]] = buckets[i].count[j] +
                    sketch[hash(buckets[i].ID[j], 101) % SKETCH_LENGTH];
            }
        }

        return ret;
    }

    void Merge(Elastic* temp){
        for(uint32_t i = 0;i < SKETCH_LENGTH;++i){
            sketch[i] += temp->sketch[i];
        }

        for(uint32_t i = 0;i < BUCKET_LENGTH;++i){
            for(uint32_t j = 0;j < COUNTER_PER_BUCKET;++j){
                int32_t minVal = 0x7fffffff;
                uint32_t minPos = 0;

                for (uint32_t k = 0; k < COUNTER_PER_BUCKET; k++){
                    if(buckets[i].ID[k] == temp->buckets[i].ID[j]){
                        buckets[i].count[k] += temp->buckets[i].count[j];
                        goto MergeEnd;
                    }

                    if(buckets[i].count[k] < minVal){
                        minPos = k;
                        minVal = buckets[i].count[k];
                    }
                }

                if((buckets[i].vote + temp->buckets[i].count[j]) >= minVal * 8){
                    buckets[i].vote = 0;

                    if(minVal != 0){
                        uint32_t position = hash(buckets[i].ID[minPos], 101) % SKETCH_LENGTH;
                        sketch[position] = sketch[position] + buckets[i].count[minPos];
                    }

                    buckets[i].ID[minPos] = temp->buckets[i].ID[j];
                    buckets[i].count[minPos] = temp->buckets[i].count[j];
                }
                else {
                    buckets[i].vote += temp->buckets[i].count[j];
                    uint32_t position = hash(temp->buckets[i].ID[j], 101) % SKETCH_LENGTH;
                    sketch[position] = sketch[position] + temp->buckets[i].count[j];
                }

                MergeEnd:
                {};
            }
        }

        delete temp;
    }

    void Merge(const Elastic_Entry<Key>& temp){
        if(temp.bucket){
            int32_t minVal = 0x7fffffff;
            uint32_t minPos = 0;

            for(uint32_t j = 0; j < COUNTER_PER_BUCKET; j++){
                if(buckets[temp.pos].ID[j] == temp.key) {
                    buckets[temp.pos].count[j] += temp.value;
                    return;
                }

                if(buckets[temp.pos].count[j] < minVal){
                    minPos = j;
                    minVal = buckets[temp.pos].count[j];
                }
            }

            if((buckets[temp.pos].vote + temp.value) >= minVal * 8){
                buckets[temp.pos].vote = 0;

                if(minVal != 0){
                    uint32_t position = hash(buckets[temp.pos].ID[minPos], 101) % SKETCH_LENGTH;
                    sketch[position] = sketch[position] + buckets[temp.pos].count[minPos];
                }

                buckets[temp.pos].ID[minPos] = temp.key;
                buckets[temp.pos].count[minPos] = temp.value;
            }
            else {
                buckets[temp.pos].vote += temp.value;
                uint32_t position = hash(temp.key, 101) % SKETCH_LENGTH;
                sketch[position] = sketch[position] + temp.value;
            }

        }
        else{
            sketch[temp.pos] += temp.value;
        }
    }
};

#endif