#!/bin/sh

readonly x='/home/admin/test.zpif'

clear
cd ./render/go
go build -o main main.go
./main "$x" n
rm main