#ifndef UTIL_H
#define UTIL_H

#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <algorithm>

#define MAX(a, b) (a > b? a:b)
#define MIN(a, b) (a < b? a:b)

typedef std::chrono::high_resolution_clock::time_point TP;

inline TP now(){
    return std::chrono::high_resolution_clock::now();
}

inline double durationms(TP finish, TP start){
    return std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
}

inline double durationns(TP finish, TP start){
    return std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
}

template<typename T>
T MEDIAN3(T array[3]){
    if(array[0] < array[1]){
        if(array[2] < array[0]){
            return array[0];
        }
        else if(array[2] < array[1]){
            return array[2];
        }
        else{
            return array[1];
        }
    }
    else{
        if(array[2] < array[1]){
            return array[1];
        }
        else if(array[2] < array[0]){
            return array[2];
        }
        else{
            return array[0];
        }
    }
}

#ifdef __linux__
static bool setaffinity(std::thread* thd, uint32_t coreId){
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreId, &cpuset);
    int rc = pthread_setaffinity_np(thd->native_handle(),
                                    sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
        return false;
    }
    return true;
}
#endif

static std::mutex mtx;

static void CDF(uint32_t* array, uint32_t length){
    double sum = 0, preSum = 0;

    for(uint32_t i = 0;i < length;++i){
        sum += array[i];
    }

    mtx.lock();
    for(uint32_t i = 0;i < length;++i){
        preSum += array[i];
        std::cout << preSum / sum << ",";
    }

    std::cout << std::endl << std::endl;
    mtx.unlock();
}

inline static void Rank(double* array, uint32_t length){
    std::sort(array, array + length);

    mtx.lock();
    for(uint32_t i = 1;i < 1000;++i){
        std::cout << array[uint32_t(0.001 * i * length)] << ",";
    }

    std::cout << std::endl << std::endl;
    mtx.unlock();
}

#endif
