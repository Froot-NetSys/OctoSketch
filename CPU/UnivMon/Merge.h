#ifndef UNIV_MERGE_H
#define UNIV_MERGE_H

#include "config.h"
#include "template/Merge.h"

template<typename Key, uint32_t thread_num>
class Univ_Merge : public Merge<Key, MyUniv<Key>*, thread_num>{
public:

    typedef ReaderWriterQueue<MyUniv<Key>*> myQueue;

    Sketch<Key>* initialize_parent(){
        return new MyUniv<Key>();
    }

    Sketch<Key>* initialize_child(){
        return initialize_parent();
    }

    Sketch<Key>* insert_child(Sketch<Key>* p, myQueue& q, const Key& packet, uint32_t number, uint32_t thread_id){
        p->insert_one(packet);

        if((number % (INTERVAL * thread_num)) == (INTERVAL * thread_id)){
            q.enqueue((MyUniv<Key>*)p);
            return initialize_child();
        }
        return p;
    }

    void merge(Sketch<Key>* p, MyUniv<Key>* temp){
        ((MyUniv<Key>*)p)->Merge(temp);
    }

};

#endif
