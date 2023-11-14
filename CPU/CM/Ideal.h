#ifndef CM_IDEAL_H
#define CM_IDEAL_H

#include "config.h"
#include "template/Ideal.h"

template<typename Key>
class CM_Ideal : public Ideal<Key>{
public:
    Sketch<Key>* initialize_sketch(){
        return new MyCM<Key>();
    }
};

#endif