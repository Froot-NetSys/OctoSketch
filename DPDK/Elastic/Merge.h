#ifndef ELASTIC_MERGE_H
#define ELASTIC_MERGE_H

#include "config.h"
#include "../template/Merge.h"

class Elastic_Merge : public Merge<uint64_t, MyElastic*>{
public:

    typedef ReaderWriterQueue<MyElastic*> myQueue;

    Sketch<uint64_t>* initialize_parent(){
        return new MyElastic();
    }

    Sketch<uint64_t>* initialize_child(){
        return initialize_parent();
    }

    Sketch<uint64_t>* insert_child(Sketch<uint64_t>* p, myQueue& q, const uint64_t& packet, uint64_t number, uint32_t thread_id){
        p->insert_one(packet);

        if((number % (INTERVAL * NUM_RX_QUEUE)) == (INTERVAL * thread_id)){
            q.enqueue((MyElastic*)p);
            return initialize_child();
        }
        return p;
    }

    void merge(Sketch<uint64_t>* p, MyElastic* temp){
        ((MyElastic*)p)->Merge(temp);
    }

};

#endif