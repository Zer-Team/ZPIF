/*
    MIT License

    Copyright (c) 2025 Zakhar Shakhanov

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

////////////////////////////////////////////////////////////////
///                       rustc 1.84.0                       ///
///                      non_snake_case                      ///
///                      minifb  0.28.0                      ///
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
///                        ID: HM0100                        ///
///                     Date: 2025-02-03                     ///
///                     Author: Zer Team                     ///
////////////////////////////////////////////////////////////////

extern crate minifb;
use std::env;
use std::fs::File;
use std::io::{self, Read};
use minifb::ScaleMode;
use minifb::{Window, WindowOptions};

// Функция для чтения `u16` в big-endian
fn convertBEInShort(bytes: &[u8]) -> u16 {
    u16::from_be_bytes([bytes[0], bytes[1]])
}

// Функция для чтения `u32` в big-endian
fn convertBEInInt(bytes: &[u8]) -> u32 {
    u32::from_be_bytes([bytes[1], bytes[2], bytes[3], bytes[4]])
}

fn main() -> io::Result<()> {
    let (mut imageWidth, mut imageHeight, mut imagePoint): (u32, u32, u64) = (0, 0, 0);

    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("\x1b[1;33mUsage: {} <file_path> [factor]\x1b[0m", args[0]);
        std::process::exit(1);
    }

    // Чтение factor из аргумента (если указан), иначе 1.0
    let factor: f64 = args.get(2).and_then(|f| f.parse().ok()).unwrap_or(1.0);
    if factor <= 0.0 {
        eprintln!("Error: factor must be greater than 0.");
        std::process::exit(1);
    }

    let mut file = File::open(&args[1])?;
    let mut buffer = [0u8; 6];

    file.read_exact(&mut buffer)?;
    if buffer != [0x89, b'Z', b'P', b'I', b'F', 0x0A] {
        eprintln!("\x1b[31mError 1: The file is damaged or the format is not supported.\x1b[0m");
        std::process::exit(1);
    }

    // Работа с чанками параметров
    while file.read_exact(&mut buffer).is_ok() {
        if buffer == [0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF] {
            break;
        }
        if buffer[0] == b'w' {
            imageWidth = convertBEInInt(&buffer);
        } else if buffer[0] == b'h' {
            imageHeight = convertBEInInt(&buffer);
        }
    }

    let mut framebuffer = vec![0u32; (imageWidth * imageHeight) as usize];

    // Окно для отрисовки
    let mut window = Window::new(
        "ZPIF Render Rust",
        (imageWidth as f64 * factor) as usize,
        (imageHeight as f64 * factor) as usize,
        WindowOptions {
            scale_mode: ScaleMode::Stretch,
            ..WindowOptions::default()
        },
    ).unwrap();

    // Работа с чанками пикселей
    while file.read_exact(&mut buffer).is_ok() {
        let quantity = convertBEInShort(&buffer);
        if buffer == [0x00, 0x00, 0x00, 0x00, 0x00, 0x00] {
            break;
        }

        let (r, g, b, _a) = (buffer[2], buffer[3], buffer[4], buffer[5]);

        for _ in 0..quantity {
            if imagePoint < (imageWidth * imageHeight) as u64 {
                framebuffer[imagePoint as usize] = ((r as u32) << 16) | ((g as u32) << 8) | (b as u32);
                imagePoint += 1;
            }
        }
    }

    // Основной цикл рендера
    while window.is_open() && !window.is_key_down(minifb::Key::Escape) {
        window.update_with_buffer(&framebuffer, imageWidth as usize, imageHeight as usize)
            .unwrap();
    }

    Ok(())
}
