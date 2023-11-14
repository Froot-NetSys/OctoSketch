#ifndef CONFIG_H
#define CONFIG_H

#include "UnivMon.h"

#define MAX_LEVEL 6

#define HASH_NUM 3
#define LENGTH (1 << 16)
#define HEAP_SIZE 0x3ff

#define INTERVAL 40000

template<typename Key>
using MyUniv = UnivMon<Key, MAX_LEVEL, HASH_NUM, LENGTH, HEAP_SIZE>;

template<typename Key>
using MyChild_Univ = Child_UnivMon<Key, MAX_LEVEL, HASH_NUM, LENGTH>;

#define THP_TIME 10

#define ALPHA 0.0002

#define PROMASK 0x7f

//#define QUEUELENGTH

#endif
