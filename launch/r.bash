#!/bin/sh

readonly path='/home/admin/test.zpif'

clear
cd ./render/r
Rscript main.r "$path"
rm Rplots.pdf