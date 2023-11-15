#ifndef XDP_CM_H
#define XDP_CM_H

#include <stdint.h>

struct Entry{
    uint64_t key;
    uint16_t hashPos;
    uint16_t pos;
    uint8_t value;
};

#endif