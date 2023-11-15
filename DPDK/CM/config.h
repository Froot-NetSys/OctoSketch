#ifndef CONFIG_H
#define CONFIG_H

#include "../../sketch/CM.h"

#define HASH_NUM 3
#define LENGTH (1 << 16)
#define HEAP_SIZE 0x3ff

#define INTERVAL 20000

using MyCM = CM<uint64_t, HASH_NUM, LENGTH, HEAP_SIZE>;

using MyChild_CM = Child_CM<uint64_t, HASH_NUM, LENGTH>;

#endif
