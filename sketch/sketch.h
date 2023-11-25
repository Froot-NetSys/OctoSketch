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

/**
 * Template for sketches
 */

template<typename Key>
class Sketch{
public:
    virtual ~Sketch(){};

    // Insert one element in the data stream
    virtual void insert_one(const Key& p) = 0;
    // Get statistics (e.g., heavy hitters) from sketches
    virtual HashMap query_all() = 0;
};

#endif