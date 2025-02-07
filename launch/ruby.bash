#!/bin/sh

readonly path='/home/admin/test.zpif'

clear
cd ./render/ruby
ruby main.rb "$path" 200
