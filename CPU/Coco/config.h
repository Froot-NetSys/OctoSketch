#ifndef CONFIG_H
#define CONFIG_H

#include "Coco.h"

#define HASH_NUM 2
#define LENGTH (1 << 16)

#define INTERVAL 100000

template<typename Key>
using MyCoco = Coco<Key, HASH_NUM, LENGTH>;

template<typename Key>
using MyChild_Coco = Child_Coco<Key, HASH_NUM, LENGTH>;

#define THP_TIME 200

#define ALPHA 0.0002

#define PROMASK 0x1f

//#define QUEUELENGTH

#endif
