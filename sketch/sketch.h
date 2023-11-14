#ifndef SKETCH_H
#define SKETCH_H

#include "../common/hash.h"
#include "../common/heap.h"

typedef std::unordered_map<uint64_t, int32_t> HashMap;

struct Packet{
    uint32_t src;
    uint32_t dst;

    operator uint64_t() const{
        return *((uint64_t*)(this));
    }
};

template<typename Key>
class Sketch{
public:
    virtual ~Sketch(){};

    virtual void insert_one(const Key& p) = 0;
    virtual HashMap query_all() = 0;
};

#endif