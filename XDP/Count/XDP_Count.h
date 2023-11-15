#ifndef XDP_COUNT_H
#define XDP_COUNT_H

#include <stdint.h>

struct Entry{
    uint64_t key;
    uint16_t hashPos;
    uint16_t pos;
    int8_t value;
};

#endif