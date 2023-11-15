#ifndef OURS_H
#define OURS_H

#include "DPDK.h"

template<typename Key, typename Entry>
class Ours{
public:

    typedef ReaderWriterQueue<Entry> myQueue;

    myQueue que[NUM_RX_QUEUE];

    virtual Sketch<Key>* initialize_parent() = 0;
    virtual Sketch<Key>* initialize_child() = 0;

    virtual void merge(Sketch<Key>* sketch, Entry temp) = 0;

    virtual void modify_threshold() = 0;

    virtual void insert_child(Sketch<Key>* sketch, myQueue& q, const Key& packet) = 0;

    void coordinator(unsigned queue_id){
        uint64_t start, end;
        uint64_t idx = 0;
        RTE_LOG(INFO, L2FWD, "entering coordinator %u\n", queue_id);

        Sketch<Key>* sketch = initialize_parent();
        Entry temp;

        while(true){
            for(uint32_t i = 0;i < NUM_RX_QUEUE;++i){
                while(que[i].try_dequeue(temp)){
                    merge(sketch, temp);
                    port_statistics[queue_id].rx += 1;

                    idx += 1;
                    if(idx > 0x3f){
                        idx = 0;
                        modify_threshold();
                    }
                }
            }
            idx += NUM_RX_QUEUE;
            if(idx > 0x3f){
                idx = 0;
                modify_threshold();
            }
        }

        delete sketch;
    }

    void local(unsigned queue_id){
        RTE_LOG(INFO, L2FWD, "%u core entering local sketch %u\n", rte_lcore_id(), queue_id);

        uint32_t batches = 0;

        struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
        struct rte_tcp_hdr *tcp[MAX_PKT_BURST];

        int sent;
        uint64_t prev_tsc, diff_tsc, cur_tsc, timer_tsc;
        unsigned portid = 0;
        unsigned nb_rx;

        prev_tsc = 0;
        timer_tsc = 0;

        Key item[MAX_PKT_BURST];
        Sketch<Key>* sketch = initialize_child();
        
        uint64_t start, end;
        uint64_t one_start, one_end;

        while(true) {
            /* Read packet from RX queues. 8< */
            nb_rx = rte_eth_rx_burst(portid, queue_id,
                            pkts_burst, MAX_PKT_BURST);

            port_statistics[queue_id].rx += nb_rx;

            for(uint32_t j = 0; j < nb_rx; j++) {
                tcp[j] = rte_pktmbuf_mtod_offset(pkts_burst[j], struct rte_tcp_hdr *,
                        sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr));
                rte_prefetch0(tcp[j]);
            }

            for(uint32_t j = 0; j < nb_rx; j++) {
                init_key(item[j], rte_be_to_cpu_16(tcp[j]->src_port), rte_be_to_cpu_16(tcp[j]->dst_port));
                rte_pktmbuf_free(pkts_burst[j]);
            }

            for(uint32_t i = 0;i < nb_rx;++i){
                insert_child(sketch, que[queue_id], item[i]);
            }

            /* >8 End of read packet from RX queues. */
        }
    }
};

#endif