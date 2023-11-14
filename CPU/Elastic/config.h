#ifndef CONFIG_H
#define CONFIG_H

#include "Elastic.h"

#define HASH_NUM 3
#define LENGTH (1 << 16)
#define HEAP_SIZE 0x3ff

#define COUNTER_PER_BUCKET 4

#define SKETCH_LENGTH (HASH_NUM * LENGTH)
#define BUCKET_LENGTH (HEAP_SIZE * 3 / COUNTER_PER_BUCKET)

#define INTERVAL 20000

template<typename Key>
using MyElastic = Elastic<Key, SKETCH_LENGTH, BUCKET_LENGTH, COUNTER_PER_BUCKET>;

template<typename Key>
using MyChild_Elastic = Child_Elastic<Key, SKETCH_LENGTH, BUCKET_LENGTH, COUNTER_PER_BUCKET>;

#define THP_TIME 200

#define ALPHA 0.0002

#define PROMASK 0x1f

//#define QUEUELENGTH

#endif
