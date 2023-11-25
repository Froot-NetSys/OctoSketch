#ifndef CONFIG_H
#define CONFIG_H

#include "CM.h"

// Configuration for the sketch
#define HASH_NUM 3
#define LENGTH (1 << 16)
#define HEAP_SIZE 0x3ff

#define INTERVAL 20000

template<typename Key>
using MyCM = CM<Key, HASH_NUM, LENGTH, HEAP_SIZE>;

template<typename Key>
using MyChild_CM = Child_CM<Key, HASH_NUM, LENGTH>;

#define THP_TIME 20

#define ALPHA 0.0002 // Threshold for finding heavy hitters

#define PROMASK 0x7f

//#define QUEUELENGTH

#endif
