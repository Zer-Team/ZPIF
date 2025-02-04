#!/bin/sh

readonly x='/home/admin/test.zpif'

clear
cd ./render/c
gcc main.c -o main -std=c99 $(pkg-config --cflags --libs gtk+-3.0) -lm
./main "$x" 100
rm main