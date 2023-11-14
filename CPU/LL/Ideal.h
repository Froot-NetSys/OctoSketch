#ifndef LL_IDEAL_H
#define LL_IDEAL_H

#include "config.h"
#include "template/Ideal.h"

template<typename Key>
class LL_Ideal : public Ideal<Key>{
public:
    Sketch<Key>* initialize_sketch(){
        return new MyLL<Key>();
    }
};

#endif