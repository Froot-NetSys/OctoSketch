CPU Code
============

Repository structure
--------------------
- `CM`: OctoSketch for the Count-Min Sketch
- `Coco`: OctoSketch for the CocoSketch
- `Count`: OctoSketch for the Count Sketch
- `Elastic`: OctoSketch for the Elastic Sketch
- `HLL`: OctoSketch for the HyperLogLog
- `LL`: OctoSketch for the LogLog
- `Locher`: OctoSketch for the Locher Sketch
- `UnivMon`: OctoSketch for the UnivMon

Requirements
-------
- cmake
- g++

How to run (example for the Count-Min Sketch)
-------
```bash
$ cd ./CM
$ cmake .
$ make
$ ./CM your-dataset
```