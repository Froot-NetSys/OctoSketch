#ifndef COCO_IDEAL_H
#define COCO_IDEAL_H

#include "config.h"
#include "template/Ideal.h"

template<typename Key>
class Coco_Ideal : public Ideal<Key>{
public:
    Sketch<Key>* initialize_sketch(){
        return new MyCoco<Key>();
    }
};

#endif