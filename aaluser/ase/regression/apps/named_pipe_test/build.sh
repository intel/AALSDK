#!/bin/sh

rm server client

rm c2s_fifo1
rm c2s_fifo2
rm s2c_fifo1


gcc -g -o server server.c
gcc -g -o client client.c

