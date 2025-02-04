#!/bin/sh

readonly path='/home/admin/Рабочий стол/ZRGE/iLove.zpif'

clear
cd ./render/cs/

dotnet build

./bin/Debug/net9.0/main "${path}" 

rm -r bin