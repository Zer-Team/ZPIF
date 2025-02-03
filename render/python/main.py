"""
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
"""

#///////////////////////////////////////////////////////////////
#//                      Pillow  11.1.0                      ///
#//                        TK  8.6.16                        ///
#//                      Python  3.13.1                      ///
#///////////////////////////////////////////////////////////////

#///////////////////////////////////////////////////////////////
#//                        ID: HM0100                        ///
#//                     Date: 2025-02-02                     ///
#//                     Author: Zer Team                     ///
#///////////////////////////////////////////////////////////////

import struct
import tkinter as tk
from tkinter import filedialog
from PIL import Image, ImageTk

# Функция для перевода 2-байтового числа (unsigned short) в формате big-endian
def convert_be_short(bytes_):
    return (bytes_[0] << 8) | bytes_[1]

# Функция для перевода 4-байтового числа (unsigned int) в формате big-endian
def convert_be_int(bytes_):
    return (bytes_[1] << 24) | (bytes_[2] << 16) | (bytes_[3] << 8) | bytes_[4]

# Функция для парсинга файла ZPIF
def parse_zpif(filename):
    with open(filename, 'rb') as f:
        data = f.read()
    
    # Проверка заголовока файла
    if data[:6] != b'\x89ZPIF\x0A':
        print("Error: The file is damaged or the format is not supported.")
        return None
    
    offset = 6
    total_bytes = len(data)
    image_width, image_height = 0, 0
    
    # Чтение параметров изображения (ширина, высота)
    while offset < total_bytes:
        chunk = data[offset:offset+6]
        if len(chunk) < 6:
            break
        
        if chunk[0] == 0x77:  # 'w' (ширина)
            image_width = convert_be_int(chunk)
        elif chunk[0] == 0x68:  # 'h' (высота)
            image_height = convert_be_int(chunk)
        elif chunk == b'\x00\x00\xff\xff\xff\xff':  # Начало данных пикселей
            break
        
        offset += 6
    
    # Проверка корректность размеров
    if image_width == 0 or image_height == 0:
        print("Error: Invalid image dimensions.")
        return None
    
    # Создание изображение RGBA
    image = Image.new('RGBA', (image_width, image_height))
    pixels = image.load()
    
    image_point = 0
    
    # Чтение данных пикселей
    while offset < total_bytes:
        chunk = data[offset:offset+6]
        if len(chunk) < 6 or chunk == b'\x00\x00\x00\x00\x00\x00':
            print("Render complete.")
            break
        
        quantity = convert_be_short(chunk)  # Количество повторяющихся пикселей
        color = (chunk[2], chunk[3], chunk[4], chunk[5])  # RGBA-значение пикселя
        
        for _ in range(quantity):
            x, y = image_point % image_width, image_point // image_width
            pixels[x, y] = color
            image_point += 1
        
        offset += 6
    
    return image

# Функция для выбора файла и его открытия
def open_file():
    file_path = filedialog.askopenfilename(filetypes=[("ZPIF files", "*.zpif")])
    if file_path:
        image = parse_zpif(file_path)
        if image:
            render_image(image)

# Функция для отображения изображения в окне Tkinter
def render_image(image):
    img_tk = ImageTk.PhotoImage(image)
    canvas.config(width=image.width, height=image.height)
    canvas.create_image(0, 0, anchor=tk.NW, image=img_tk)
    canvas.image = img_tk  # Сохранение ссылки, чтобы изображение не удалялось сборщиком мусора

# Создание GUI-интерфейса с помощью Tkinter
root = tk.Tk()
root.title("ZPIF Render Python")

# Создание холста для отображения изображения
canvas = tk.Canvas(root)
canvas.pack()

# Кнопка для выбора файла
btn = tk.Button(root, text="Open ZPIF File", command=open_file)
btn.pack()

# Главный цикл Tkinter
root.mainloop()