#include "../xdp_common/hash.h"
#include "../xdp_common/parse.h"

#include "XDP_CM.h"

#define HASH_NUM 3
#define LENGTH 65536

struct Length{
    uint64_t nanoseconds;
    uint64_t length;
};

struct{
    __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
    __type(key, __u32);
    __type(value, uint8_t);
    __uint(max_entries, HASH_NUM * LENGTH);
} sketch SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
    __type(key, __u32);
    __type(value, uint64_t);
    __uint(max_entries, 1);
} packets SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __type(key, __u32);
    __type(value, int32_t);
    __uint(max_entries, 1);
} threshold SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 64 * 1024);
} buffer SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __type(key, __u32);
    __type(value, struct Length);
    __uint(max_entries, 1);
} buffer_length SEC(".maps");


SEC("xdp")
int xdp_prog(struct xdp_md *skb)
{
    uint64_t key;
    
    if(parse_key(skb, (struct Packet*)&key) < 0)
        return XDP_DROP;

    uint64_t *number = bpf_map_lookup_elem(&packets, &zero);
    if(number){
        *number += 1;
        if(((*number) & 0xff) == 0xff){
            struct Length *len = bpf_map_lookup_elem(&buffer_length, &zero);
            if(len){
                uint64_t ns = bpf_ktime_get_ns();
                if(ns > len->nanoseconds + 100000){
                    len->nanoseconds = ns;
                    len->length = bpf_ringbuf_query(&buffer, BPF_RB_AVAIL_DATA);
                }
            }
        }
    }

    uint32_t index[HASH_NUM];
    int32_t *thd = bpf_map_lookup_elem(&threshold, &zero);

    if(!thd){
        return XDP_DROP;
    }
        
    for(uint32_t i = 0;i < HASH_NUM;++i){
        index[i] = hash(key, seed[i]) % (uint32_t)LENGTH + i * LENGTH;
    }

    for(uint32_t i = 0;i < HASH_NUM;++i){
        uint8_t* counter = bpf_map_lookup_elem(&sketch, &index[i]);
        if(counter){
            *counter += 1;
            if(*counter > *thd){
                struct Entry *entry = bpf_ringbuf_reserve(&buffer, sizeof(struct Entry), 0);
                if(entry){
                    entry->key = key;
                    entry->hashPos = i;
                    entry->pos = index[i];
                    entry->value = *counter;
                    bpf_ringbuf_submit(entry, 0);
                    *counter = 0;
                }
            }
        }
    }
        
    return XDP_DROP;
}

char _license[] SEC("license") = "GPL";
