#ifndef XDP_PARSE_H
#define XDP_PARSE_H

#include <linux/tcp.h>
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>

struct Packet{
    uint32_t src;
    uint32_t dst;
};

inline void init_key(struct Packet* key, uint64_t src, uint64_t dst){
    key->src = src;
    key->dst = dst;
}

inline int32_t parse_key(struct xdp_md *skb, struct Packet* key){
    void *data_end = (void *)(long)skb->data_end;
    void *data = (void *)(unsigned long long)skb->data;

    if(data + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr) > data_end)
        return -1;
    
    struct iphdr *iph = data + sizeof(struct ethhdr);
    if(iph->protocol == IPPROTO_TCP){
        struct tcphdr *h = ((void*)iph) + sizeof(struct iphdr);
        init_key(key, h->source, h->dest);
        return 0;
    }

    return -1;
}

#endif