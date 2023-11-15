#ifndef XDP_HASH_H
#define XDP_HASH_H

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX(a, b) (a > b? a:b)
#define MIN(a, b) (a < b? a:b)

static const uint32_t zero = 0;
static const uint32_t seed[] = {8017, 3593, 1427};
static const uint32_t Prime[] = {
        2654435761U,246822519U,3266489917U,668265263U,374761393U};

uint32_t rotateLeft(uint32_t x, uint8_t bits){
    return (x << bits) | (x >> (32 - bits));
}

uint32_t hash(uint64_t data, uint32_t seed){
    uint32_t state[4] = {seed + Prime[0] + Prime[1],
                         seed + Prime[1], seed, seed - Prime[0]};
    uint32_t result = sizeof(uint64_t) + state[2] + Prime[4];

    result = rotateLeft(result + (data & 0xffffffff) * Prime[2], 17) * Prime[3];
    result = rotateLeft(result + (data >> 32) * Prime[2], 17) * Prime[3];

    result ^= result >> 15;
    result *= Prime[1];
    result ^= result >> 13;
    result *= Prime[2];
    result ^= result >> 16;
    return result;
}

#endif