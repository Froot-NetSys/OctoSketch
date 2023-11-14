#ifndef COUNT_IDEAL_H
#define COUNT_IDEAL_H

#include "config.h"
#include "template/Ideal.h"

template<typename Key>
class Count_Ideal : public Ideal<Key>{
public:
    Sketch<Key>* initialize_sketch(){
        return new MyCount<Key>();
    }
};


#endif