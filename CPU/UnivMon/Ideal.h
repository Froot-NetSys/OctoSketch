#ifndef UNIVMON_IDEAL_H
#define UNIVMON_IDEAL_H

#include "config.h"
#include "template/Ideal.h"

template<typename Key>
class Univ_Ideal : public Ideal<Key>{
public:
    Sketch<Key>* initialize_sketch(){
        return new MyUniv<Key>();
    }
};


#endif