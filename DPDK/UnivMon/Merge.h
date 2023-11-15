#ifndef UNIV_MERGE_H
#define UNIV_MERGE_H

#include "config.h"
#include "../template/Merge.h"

class Univ_Merge : public Merge<uint64_t, MyUniv*>{
public:

    typedef ReaderWriterQueue<MyUniv*> myQueue;

    Sketch<uint64_t>* initialize_parent(){
        return new MyUniv();
    }

    Sketch<uint64_t>* initialize_child(){
        return initialize_parent();
    }

    Sketch<uint64_t>* insert_child(Sketch<uint64_t>* p, myQueue& q, const uint64_t& packet, uint64_t number, uint32_t thread_id){
        p->insert_one(packet);

        if((number % (INTERVAL * NUM_RX_QUEUE)) == (INTERVAL * thread_id)){
            q.enqueue((MyUniv*)p);
            return initialize_child();
        }
        return p;
    }

    void merge(Sketch<uint64_t>* p, MyUniv* temp){
        ((MyUniv*)p)->Merge(temp);
    }

};

#endif