#ifndef COCO_MERGE_H
#define COCO_MERGE_H

#include "config.h"
#include "template/Merge.h"

template<typename Key, uint32_t thread_num>
class Coco_Merge : public Merge<Key, MyCoco<Key>*, thread_num>{
public:

    typedef ReaderWriterQueue<MyCoco<Key>*> myQueue;

    Sketch<Key>* initialize_parent(){
        return new MyCoco<Key>();
    }

    Sketch<Key>* initialize_child(){
        return initialize_parent();
    }

    Sketch<Key>* insert_child(Sketch<Key>* p, myQueue& q, const Key& packet, uint32_t number, uint32_t thread_id){
        p->insert_one(packet);

        if((number % (INTERVAL * thread_num)) == (INTERVAL * thread_id)){
            q.enqueue((MyCoco<Key>*)p);
            return initialize_child();
        }
        return p;
    }

    void merge(Sketch<Key>* p, MyCoco<Key>* temp){
        ((MyCoco<Key>*)p)->Merge(temp);
    }

};

#endif
