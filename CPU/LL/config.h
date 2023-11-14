#ifndef CONFIG_H
#define CONFIG_H

#include "LL.h"

#define LENGTH (1 << 16)

#define INTERVAL 10000

template<typename Key>
using MyLL = LL<Key, LENGTH>;

template<typename Key>
using MyChild_LL = Child_LL<Key, LENGTH>;

#define THP_TIME 10

#define ALPHA 0

#define PROMASK 0x0

//#define QUEUELENGTH

#endif
