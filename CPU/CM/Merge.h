#ifndef CM_MERGE_H
#define CM_MERGE_H

#include "config.h"
#include "template/Merge.h"

template<typename Key, uint32_t thread_num>
class CM_Merge : public Merge<Key, MyCM<Key>*, thread_num>{
public:

    typedef ReaderWriterQueue<MyCM<Key>*> myQueue;

    Sketch<Key>* initialize_parent(){
        return new MyCM<Key>();
    }

    Sketch<Key>* initialize_child(){
        return initialize_parent();
    }

    Sketch<Key>* insert_child(Sketch<Key>* p, myQueue& q, const Key& packet, uint32_t number, uint32_t thread_id){
        p->insert_one(packet);

        if((number % (INTERVAL * thread_num)) == (INTERVAL * thread_id)){
            q.enqueue((MyCM<Key>*)p);
            return initialize_child();
        }
        return p;
    }

    void merge(Sketch<Key>* p, MyCM<Key>* temp){
        ((MyCM<Key>*)p)->Merge(temp);
    }

};

#endif