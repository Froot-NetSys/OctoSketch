#ifndef IDEAL_H
#define IDEAL_H

#include "Abstract.h"

template<typename Key>
class Ideal : public Abstract{
public:

    virtual Sketch<Key>* initialize_sketch() = 0;

    void update(void* start, uint32_t size, HashMap mp){
        uint32_t length = size  / sizeof(Key);
        Key* dataset = (Key*)start;

        Sketch<Key>* sketch = initialize_sketch();
        
        for(uint32_t i = 0;i < length;++i){
            sketch->insert_one(dataset[i]);
        }

        std::cout << "Baseline Accuracy:" << std::endl;

        HashMap ret = sketch->query_all();
        HHCompare(ret, mp, length * ALPHA);

        delete sketch;
    }
};

#endif
