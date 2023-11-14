#ifndef CONFIG_H
#define CONFIG_H

#include "Count.h"

#define HASH_NUM 3
#define LENGTH (1 << 16)
#define HEAP_SIZE 0x3ff

#define INTERVAL 20000

template<typename Key>
using MyCount = Count<Key, HASH_NUM, LENGTH, HEAP_SIZE>;

template<typename Key>
using MyChild_Count = Child_Count<Key, HASH_NUM, LENGTH>;

#define THP_TIME 200

#define ALPHA 0.0002

#define PROMASK 0x7f

//#define QUEUELENGTH

#endif
