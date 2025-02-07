#!/bin/sh

readonly path='/home/admin/test.zpif'

clear
cd ./compression
g++ compression.cpp -o main -std=c++20
./main "$path" 'out.zpif'
rm main