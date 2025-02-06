#!/bin/sh

readonly path='/home/admin/test.zpif'

clear
cd ./render/go
go build -o main main.go
./main "$path" n
rm main