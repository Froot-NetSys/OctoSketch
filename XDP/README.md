XDP Integration
============

Environment
--------------------
We have tested on
 - xdp-tools-1.2.9
 - Linux kernel version 5.15.0

Repository structure
--------------------
- `CM`: OctoSketch for the Count-Min Sketch
- `Coco`: OctoSketch for the CocoSketch
- `Count`: OctoSketch for the Count Sketch
- `xdp_common`: Common functions used in OctoSketch of XDP

How to run (example for the Count-Min Sketch)
-------
```bash
$ cd ./CM
$ sudo ./compile.sh
$ sudo ./main
```