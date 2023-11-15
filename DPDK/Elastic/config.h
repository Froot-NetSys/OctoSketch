#ifndef CONFIG_H
#define CONFIG_H

#include "../../sketch/Elastic.h"

#define HASH_NUM 3
#define LENGTH (1 << 16)
#define HEAP_SIZE 0x3ff

#define COUNTER_PER_BUCKET 4

#define SKETCH_LENGTH (HASH_NUM * LENGTH)
#define BUCKET_LENGTH (HEAP_SIZE * 3 / COUNTER_PER_BUCKET)

#define INTERVAL 20000

using MyElastic = Elastic<uint64_t, SKETCH_LENGTH, BUCKET_LENGTH, COUNTER_PER_BUCKET>;

using MyChild_Elastic = Child_Elastic<uint64_t, SKETCH_LENGTH, BUCKET_LENGTH, COUNTER_PER_BUCKET>;

#endif
