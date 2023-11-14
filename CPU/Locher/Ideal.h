#ifndef LOCHER_IDEAL_H
#define LOCHER_IDEAL_H

#include "config.h"
#include "template/Ideal.h"

class Locher_Ideal : public Ideal<Packet>{
public:
    Sketch<Packet>* initialize_sketch(){
        return new MyLocher();
    }
};

#endif