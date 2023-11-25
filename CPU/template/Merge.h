#ifndef MERGE_H
#define MERGE_H

#include "Abstract.h"

/**
 * Template for sketch-merge running in multiple cores
 */

template<typename Key, typename Entry, uint32_t thread_num>
class Merge: public Abstract{
public:

    typedef ReaderWriterQueue<Entry> myQueue;

    myQueue que[thread_num];

    virtual Sketch<Key>* initialize_parent() = 0;
    virtual Sketch<Key>* initialize_child() = 0;

    virtual void merge(Sketch<Key>* sketch, Entry temp) = 0;

    virtual Sketch<Key>* insert_child(Sketch<Key>* sketch, myQueue& q, const Key& packet, uint32_t number, uint32_t thread_id) = 0;

    void update(void* start, uint32_t size, HashMap mp){
        std::thread parent;
        parent = std::thread(&Merge::ParentThread, this, &parent, start, size, &mp);
        parent.join();
    }

    /**
     * The thread of the aggregator
     */
    void ParentThread(std::thread* thisThd, void* start, uint32_t size, HashMap* mp){
#ifdef __linux__
        if(!setaffinity(thisThd, thread_num))
            return;
#endif
        std::atomic<int32_t> finish(0);
        std::thread thd[thread_num];

        Sketch<Key>* sketch = initialize_parent();

        for(uint32_t i = 0;i < thread_num;++i){
            thd[i] = std::thread(&Merge::ChildThread, this,
                &(thd[i]), i, start, size, &finish);
        }

        collect(sketch, finish);

        for(uint32_t i = 0;i < thread_num;++i){
            thd[i].join();
        }

        std::cout << "Merge Accuracy:" << std::endl;
        HashMap ret = sketch->query_all();
        HHCompare(ret, (*mp), size / sizeof(Key) * ALPHA);

        delete sketch;
    }

    /**
     * The thread of each worker
     */
    void ChildThread(std::thread* thisThd, uint32_t thread_id, void* start, uint32_t size,
            std::atomic<int32_t>* finish)
            {
#ifdef __linux__
        if(!setaffinity(thisThd, thread_id))
            return;
#endif
        Sketch<Key>* sketch = initialize_child();

        std::vector<Key> dataset;

        Partition<Key, thread_num>((Key*)start, size / sizeof(Key), thread_id, dataset);

        sketch = insert(dataset, sketch, que[thread_id], thread_id);
        
        (*finish)++;
        delete sketch;
    }

    Sketch<Key>* insert(const std::vector<Key>& dataset, Sketch<Key>* sketch, myQueue& q, uint32_t thread_id){
        uint32_t number = 1, length = dataset.size();

        for(uint32_t i = 0;i < length;++i){
            sketch = insert_child(sketch, q, dataset[i], number, thread_id);
            number += 1;
        }
        return sketch;
    }

    void collect(Sketch<Key>* sketch, std::atomic<int32_t>& finish){
        Entry temp;

        while(finish < thread_num){
#ifdef QUEUELENGTH
            uint32_t total = 0;
            for(uint32_t i = 0;i < thread_num;++i){
                total += que[i].size_approx(); 
            }
            if(total > 1){
                std::cout << "Merge Delay: " << total << std::endl;
            }
#endif
            for(uint32_t i = 0;i < thread_num;++i){
                if(que[i].try_dequeue(temp)){
                    merge(sketch, temp);                 
                }
            }
        }

        for(uint32_t i = 0;i < thread_num;++i){
            while(que[i].try_dequeue(temp)){
                merge(sketch, temp);
            }
        }
    }

};

#endif