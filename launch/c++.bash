#!/bin/sh

readonly path='/home/admin/test.zpif'

clear
cd ./render/c++
g++ main.cpp -o main -lsfml-graphics -lsfml-window -lsfml-system -std=c++20
./main "$path"
rm main