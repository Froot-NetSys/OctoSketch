#ifndef XDP_COCO_H
#define XDP_COCO_H

#include <stdbool.h>
#include <stdint.h>

struct Entry{
    uint64_t key;
    uint16_t pos[2];
    uint8_t value;
    bool pos_valid;
};

#endif