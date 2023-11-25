#ifndef ABSTRACT_H
#define ABSTRACT_H

#include "sketch.h"
#include "readerwriterqueue.h"

#include <thread>
#include <unordered_map>

typedef std::unordered_map<uint64_t, int32_t> HashMap;

class Abstract{
public:

    virtual void update(void* start, uint32_t size, HashMap mp) = 0;
    virtual ~Abstract(){};

    template<typename Key, uint32_t thread_num>
    static void Partition(Key* start, uint32_t size, uint32_t id, std::vector<Key>& vec){
        for(uint32_t i = 0;i < size;++i){
            if(hash(start[i], 101) % thread_num == id){
                vec.push_back(start[i]);
            }
        }
    }

    static void HHCompare(HashMap test, HashMap real, int32_t threshold){
        double estHH = 0, HH = 0, both = 0;
        double CR = 0, PR = 0, AAE = 0, ARE = 0;

        for(auto it = test.begin();it != test.end();++it){
            if(it->second > threshold){
                estHH += 1;
                if(real[it->first] > threshold){
                    both += 1;
                    AAE += abs(real[it->first] - it->second);
                    ARE += abs(real[it->first] - it->second) / (double)real[it->first];
                }
            }
        }

        for(auto it = real.begin();it != real.end();++it){
            if(it->second > threshold){
                HH += 1;
            }
        }
        
        std::cout << "CR: " << both / HH << std::endl
                << "PR: " << both / estHH << std::endl
                << "AAE: " << AAE / both << std::endl
                << "ARE: " << ARE / both << std::endl << std::endl;
    }
};

#endif