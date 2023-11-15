#include "../xdp_common/hash.h"
#include "../xdp_common/parse.h"

#include "XDP_Coco.h"

#define HASH_NUM 2
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
} counters SEC(".maps");

struct{
    __uint(type, BPF_MAP_TYPE_PERCPU_ARRAY);
    __type(key, __u32);
    __type(value, uint64_t);
    __uint(max_entries, HASH_NUM * LENGTH);
} keys SEC(".maps");

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

int32_t fill_entry(uint64_t _key, uint16_t _pos0, uint16_t _pos1, 
                    uint8_t _value, bool _pos_valid){
    struct Entry *entry = bpf_ringbuf_reserve(&buffer, sizeof(struct Entry), 0);
    if(entry){
        entry->key = _key;
        entry->pos[0] = _pos0;
        entry->pos[1] = _pos1;
        entry->value = _value;
        bpf_ringbuf_submit(entry, 0);
    }
    return 0;
}

SEC("xdp")
int xdp_prog(struct xdp_md *skb)
{
    uint64_t packet;
    
    if(parse_key(skb, (struct Packet*)&packet) < 0)
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

    int32_t *thd = bpf_map_lookup_elem(&threshold, &zero);

    if(!thd){
        return XDP_DROP;
    }

    uint32_t choice;
    uint32_t hashNum = hash(packet, seed[0]);
    uint32_t pos[2] = {hashNum >> 16, (hashNum & 0xffff) + LENGTH};

    uint64_t* key[2];
    uint8_t* counter[2];

    key[0] = bpf_map_lookup_elem(&keys, &pos[0]);
    counter[0] = bpf_map_lookup_elem(&counters, &pos[0]);

    if(!key[0] || !counter[0])
        return XDP_DROP;
        
    if(*key[0] == packet){
        *counter[0] += 1;
        if(*counter[0] > *thd){
            fill_entry(packet, pos[0], pos[1] - LENGTH, *counter[0], true);
            *counter[0] = 0;
        }
        return XDP_DROP;
    }


    key[1] = bpf_map_lookup_elem(&keys, &pos[1]);
    counter[1] = bpf_map_lookup_elem(&counters, &pos[1]);
        
    if(!key[1] || !counter[1])
        return XDP_DROP;

    if(*key[1] == packet){
        *counter[1] += 1;
        if(*counter[1] > *thd){
            fill_entry(packet, pos[0], pos[1] - LENGTH, *counter[1], true);
            *counter[1] = 0;
        }
        return XDP_DROP;
    }

    choice = (*counter[0] > *counter[1]);
    uint64_t* tmp_key = key[choice];
    uint8_t* tmp_counter = counter[choice];

    *tmp_counter += 1;

    if(bpf_get_prandom_u32() % (*tmp_counter) == 0){
        *tmp_key = packet;
        if(*tmp_counter > *thd){
            fill_entry(packet, pos[0], pos[1] - LENGTH, *tmp_counter, true);
            *tmp_counter = 0;
        }
    }
    else{
        if(*tmp_counter > *thd){
            fill_entry(*tmp_key, pos[0], pos[1] - LENGTH, *tmp_counter, false);
            *tmp_counter = 0;
        }
    }
        
    return XDP_DROP;
}

char _license[] SEC("license") = "GPL";
