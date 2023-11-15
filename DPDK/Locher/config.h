#ifndef CONFIG_H
#define CONFIG_H

#include "../../sketch/Locher.h"

#define HASH_NUM 3
#define LENGTH (1 << 16)
#define HEAP_SIZE 0xff

#define INTERVAL 60000

using MyLocher = Locher<HASH_NUM, LENGTH, HEAP_SIZE>;

using MyChild_Locher = Child_Locher<HASH_NUM, LENGTH>;

#define HARDTHRES 0x11

#endif
