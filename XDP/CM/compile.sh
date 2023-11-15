clang -O2 -g -Wall -target bpf -c xdp.c -o xdp.o
g++ main.c -o main -lbpf -lxdp

sudo /local/xdp-tools/xdp-loader/xdp-loader unload -a enp65s0f0np0