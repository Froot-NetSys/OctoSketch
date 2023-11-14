#ifndef HLL_MERGE_H
#define HLL_MERGE_H

#include "config.h"
#include "template/Merge.h"

template<typename Key, uint32_t thread_num>
class HLL_Merge : public Merge<Key, MyHLL<Key>*, thread_num>{
public:

    typedef ReaderWriterQueue<MyHLL<Key>*> myQueue;

    Sketch<Key>* initialize_parent(){
        return new MyHLL<Key>();
    }

    Sketch<Key>* initialize_child(){
        return initialize_parent();
    }

    Sketch<Key>* insert_child(Sketch<Key>* p, myQueue& q, const Key& packet, uint32_t number, uint32_t thread_id){
        p->insert_one(packet);

        if((number % (INTERVAL * thread_num)) == (INTERVAL * thread_id)){
            q.enqueue((MyHLL<Key>*)p);
            return initialize_child();
        }
        return p;
    }

    void merge(Sketch<Key>* p, MyHLL<Key>* temp){
        ((MyHLL<Key>*)p)->Merge(temp);
    }

};

#endif
