#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <iostream>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_string_fns.h>

#include "../../common/util.h"
#include "../../sketch/sketch.h"
#include "../../queue/readerwriterqueue.h"


#define NUM_RX_QUEUE 10

#define UPPER_LENGTH 0xa0
#define TARGET_LENGTH 0x80
#define LOWER_LENGTH 0x60

static weak_atomic<bool> force_quit;

#define RTE_LOGTYPE_L2FWD RTE_LOGTYPE_USER1

#define MAX_PKT_BURST 32
#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */
#define MEMPOOL_CACHE_SIZE 256

/*
 * Configurable number of RX/TX ring descriptors
 */
#define RTE_TEST_RX_DESC_DEFAULT 1024
#define RTE_TEST_TX_DESC_DEFAULT 1024
static uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

/* ethernet addresses of ports */
static struct rte_ether_addr l2fwd_ports_eth_addr[RTE_MAX_ETHPORTS];

/* mask of enabled ports */
static uint32_t l2fwd_enabled_port_mask = 0x1;

/* list of enabled ports */

static unsigned int l2fwd_rx_queue_per_lcore = 1;

#define MAX_RX_QUEUE_PER_LCORE 16
#define MAX_TX_QUEUE_PER_PORT 16

static struct rte_eth_dev_tx_buffer *tx_buffer[RTE_MAX_ETHPORTS];

static struct rte_eth_conf port_conf;

struct rte_mempool * l2fwd_pktmbuf_pool = NULL;

/* Per-port statistics struct */
struct l2fwd_port_statistics {
	uint64_t rx;
    uint64_t poll_cycles;
    uint64_t sketch_cycles;
} __rte_cache_aligned;
struct l2fwd_port_statistics port_statistics[NUM_RX_QUEUE + 1];

#define MAX_TIMER_PERIOD 86400 /* 1 day max */
/* A tsc-based timer responsible for triggering statistics printout */
static uint64_t timer_period = 10; /* default period is 10 seconds */

inline void init_key(uint64_t& key, uint64_t src, uint64_t dst){
    key = ((src << 32) | dst);
}

inline void init_key(Packet& packet, uint64_t src, uint64_t dst){
    packet.src = src;
    packet.dst = dst;
}

/* Print out statistics */
void
print_stats()
{
	uint64_t old_total_packets_rx = 0, total_packets_rx = 0;
    uint64_t old_total_core_rx = 0, total_core_rx = 0;

	uint32_t portid = 0, i;
	uint64_t t, last_time = rte_get_tsc_cycles(), intervalMs;

	const char clr[] = { 27, '[', '2', 'J', '\0' };
	const char topLeft[] = { 27, '[', '1', ';', '1', 'H','\0' };

	while(true){
	    t = rte_get_tsc_cycles();
	    intervalMs = 1000000 * (t - last_time) / (rte_get_tsc_hz());

	    if(intervalMs >= 1000000){
	        total_packets_rx = 0;
            total_core_rx = port_statistics[NUM_RX_QUEUE].rx;
            for(i = 0;i < NUM_RX_QUEUE;++i){
                total_packets_rx += port_statistics[i].rx;
            }
            last_time = rte_get_tsc_cycles();
            
            printf("Port statistics ====================================\n");
            printf("Time: %f\n", (double)t / rte_get_tsc_hz());

            printf("Packets received: %lu\n",
                    total_packets_rx - old_total_packets_rx);
            printf("Core Entry received: %lu\n",
                    total_core_rx - old_total_core_rx);

            printf("\n\n");
            fflush(stdout);

            old_total_packets_rx = total_packets_rx;
            old_total_core_rx = total_core_rx;
	    }
	}
}

#endif
