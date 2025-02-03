#!/bin/sh

readonly x='/home/admin/test.zpif'

clear
cd ./render/rust
rustc -A non_snake_case main.rs -L /home/admin/Рабочий\ стол/ZPIF/render/rust/rust_minifb/target/release/deps --extern minifb=/home/admin/Рабочий\ стол/ZPIF/render/rust/rust_minifb/target/release/libminifb.rlib
./main "$x" 100
rm main