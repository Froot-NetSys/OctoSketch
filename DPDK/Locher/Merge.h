#ifndef LOCHER_MERGE_H
#define LOCHER_MERGE_H

#include "config.h"
#include "../template/Merge.h"

class Locher_Merge : public Merge<Packet, MyLocher*>{
public:

    typedef ReaderWriterQueue<MyLocher*> myQueue;

    Sketch<Packet>* initialize_parent(){
        return new MyLocher();
    }

    Sketch<Packet>* initialize_child(){
        return initialize_parent();
    }

    Sketch<Packet>* insert_child(Sketch<Packet>* p, myQueue& q, const Packet& packet, uint64_t number, uint32_t thread_id){
        p->insert_one(packet);

        if((number % (INTERVAL * NUM_RX_QUEUE)) == (INTERVAL * thread_id)){
            q.enqueue((MyLocher*)p);
            return initialize_child();
        }
        return p;
    }

    void merge(Sketch<Packet>* p, MyLocher* temp){
        ((MyLocher*)p)->Merge(temp);
    }

};

#endif