DPDK Integration
============

Environment
--------------------
We have tested on
 - DPDK 21.11
 - Linux kernel version 5.15.0-47-generic

Repository structure
--------------------
- `CM`: OctoSketch for the Count-Min Sketch
- `Coco`: OctoSketch for the CocoSketch
- `Count`: OctoSketch for the Count Sketch
- `Elastic`: OctoSketch for the Elastic Sketch
- `Locher`: OctoSketch for the Locher Sketch
- `UnivMon`: OctoSketch for the UnivMon
- `template`: Common functions used in OctoSketch of DPDK

How to run (example for the Count-Min Sketch)
-------
```bash
$ cd ./CM
$ make
$ sudo ./build/OctoSketch -l 0-12 -n 4
```