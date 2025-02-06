#!/bin/sh

readonly path='/home/admin/test.zpif'

clear
cd ./render/c
gcc main.c -o main -std=c99 $(pkg-config --cflags --libs gtk+-3.0) -lm
./main "$path" 100
rm main