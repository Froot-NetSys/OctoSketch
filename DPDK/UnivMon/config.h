#ifndef CONFIG_H
#define CONFIG_H

#include "../../sketch/UnivMon.h"

#define MAX_LEVEL 6

#define HASH_NUM 3
#define LENGTH (1 << 16)
#define HEAP_SIZE 0x3ff

#define INTERVAL 40000

using MyUniv = UnivMon<uint64_t, MAX_LEVEL, HASH_NUM, LENGTH, HEAP_SIZE>;

using MyChild_Univ = Child_UnivMon<uint64_t, MAX_LEVEL, HASH_NUM, LENGTH>;

#endif
