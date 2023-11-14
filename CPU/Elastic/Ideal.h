#ifndef ELASTIC_IDEAL_H
#define ELASTIC_IDEAL_H

#include "config.h"
#include "template/Ideal.h"

template<typename Key>
class Elastic_Ideal : public Ideal<Key>{
public:
    Sketch<Key>* initialize_sketch(){
        return new MyElastic<Key>();
    }
};

#endif