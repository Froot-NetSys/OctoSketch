#ifndef HLL_IDEAL_H
#define HLL_IDEAL_H

#include "config.h"
#include "template/Ideal.h"

template<typename Key>
class HLL_Ideal : public Ideal<Key>{
public:
    Sketch<Key>* initialize_sketch(){
        return new MyHLL<Key>();
    }
};

#endif