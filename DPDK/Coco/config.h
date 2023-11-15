#ifndef CONFIG_H
#define CONFIG_H

#include "../../sketch/Coco.h"

#define HASH_NUM 2
#define LENGTH (1 << 16)

#define INTERVAL 100000

using MyCoco = Coco<uint64_t, HASH_NUM, LENGTH>;

using MyChild_Coco = Child_Coco<uint64_t, HASH_NUM, LENGTH>;

#endif
