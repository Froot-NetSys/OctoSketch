#include "Merge.h"
#include "Ours.h"

#include "../template/DPDK.h"

#define MYSKETCH Locher_Ours

std::atomic<bool> is_used[NUM_RX_QUEUE + 1];

int distribute(void *arg){
    MYSKETCH* sketch = (MYSKETCH*) arg;

    if(rte_get_main_lcore() == rte_lcore_id()){
        print_stats();
        return 1;
    }

    bool set = true;

    set = is_used[0].exchange(set);
    if(!set){
        sketch->coordinator(NUM_RX_QUEUE);
        return 2;
    }

    for(uint32_t i = 0;i < NUM_RX_QUEUE;++i){
        set = is_used[i + 1].exchange(set);
        if(!set){
            sketch->local(i);
            return 3;
        }
    }

    RTE_LOG(INFO, L2FWD, "nothing to do for lcore %u\n", rte_lcore_id());

    return 0;
}

int main(int argc, char **argv)
{
    main_dpdk(argc, argv);

	MYSKETCH* sketch = new MYSKETCH();

    int ret = rte_eal_mp_remote_launch(distribute, sketch, CALL_MAIN);

	return ret;
}
