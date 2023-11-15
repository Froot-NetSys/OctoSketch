#ifndef XDP_SHLL_H
#define XDP_SHLL_H

#include <stdint.h>

struct Buckets{
    uint8_t counter0 : 4;
    uint8_t counter1 : 4;
};

struct SHLL{
    struct Buckets buckets[8];
};

#endif