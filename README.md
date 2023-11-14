# OctoSketch: Enabling Real-Time, Continuous Network Monitoring over Multiple Cores

## Introduction

**OctoSketch** is a software monitoring framework that can scale a wide spectrum of sketches to many cores with high online accuracy and performance. In contrast to previous systems that adopt straightforward sketch merges from individual cores to obtain the aggregated result, we devise a continuous, change-based mechanism that can generally be applied to sketches to perform the aggregation. This design ensures high online accuracy of the aggregated result at any query time and reduces computation costs to achieve high throughput. We apply OctoSketch to nine representative sketches on three software platforms (CPU, DPDK, and eBPF XDP). Our results demonstrate that OctoSketch achieves about 15.6× lower errors and up to 4.5× higher throughput than the state-of-the-art.

[*Our paper*](https://yindazhang.github.io/files/OctoSketch.pdf) will appear in NSDI 2024.

## About this repo

- `CPU`: OctoSketch implemented on CPU
- `DPDK`: OctoSketch implemented on DPDK
- `XDP`: OctoSketch implemented on eBPF XDP
- `common`: Common functions used by OctoSketch
- `queue`: Concurrent queues used by OctoSketch
- `sketch`: Sketches applied in OctoSketch
- more details can be found in folders.