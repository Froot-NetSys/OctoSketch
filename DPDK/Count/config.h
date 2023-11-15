#ifndef CONFIG_H
#define CONFIG_H

#include "../../sketch/Count.h"

#define HASH_NUM 3
#define LENGTH (1 << 16)
#define HEAP_SIZE 0x3ff

#define INTERVAL 20000

using MyCount = Count<uint64_t, HASH_NUM, LENGTH, HEAP_SIZE>;

using MyChild_Count = Child_Count<uint64_t, HASH_NUM, LENGTH>;

#endif
