=begin
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
=end

#///////////////////////////////////////////////////////////////
#//                        ruby 3.3.5                        ///
#//                       gosu (1.4.6)                       ///
#//                           RUBY                           ///
#///////////////////////////////////////////////////////////////

#///////////////////////////////////////////////////////////////
#//                        ID: HM0100                        ///
#//                     Date: 2025-02-07                     ///
#//                     Author: Zer Team                     ///
#///////////////////////////////////////////////////////////////

require 'gosu'

class ZPIFRenderer < Gosu::Window
  def initialize(file_path, factor = 1)
    @factor = factor.clamp(1, 65535) # Ограничение коэффициента увеличения
    @image_data = load_zpif(file_path) # Загрузка изображения из ZPIF

    super(@image_data[:width] * @factor, @image_data[:height] * @factor, false)
    self.caption = "ZPIF Renderer Ruby"
  end

  # Функция загрузки ZPIF
  def load_zpif(file_path)
    file = File.open(file_path, 'rb')

    # Читаем заголовок (6 байт)
    header = file.read(6)
    unless header == [0x89, 'Z'.ord, 'P'.ord, 'I'.ord, 'F'.ord, 0x0A].pack('C*')
      raise "Error: Invalid ZPIF file"
    end

    width = height = 0
    pixels = []
    pixel_point = 0

    # Читаем параметры (ширина, высота)
    while (chunk = file.read(6))
      bytes = chunk.bytes

      break if bytes == [0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF] # Конец параметров

      case bytes[0]
      when 'w'.ord
        width = (bytes[1] << 24) | (bytes[2] << 16) | (bytes[3] << 8) | bytes[4]
      when 'h'.ord
        height = (bytes[1] << 24) | (bytes[2] << 16) | (bytes[3] << 8) | bytes[4]
      end
    end

    raise "Error: Invalid image size" if width <= 0 || height <= 0

    # Читаем пиксели
    while (chunk = file.read(6))
      bytes = chunk.bytes
      break if bytes == [0x00, 0x00, 0x00, 0x00, 0x00, 0x00] # Конец пикселей

      quantity = (bytes[0] << 8) | bytes[1] # Количество пикселей
      color = Gosu::Color.rgba(bytes[2], bytes[3], bytes[4], bytes[5])

      quantity.times do
        pixels[pixel_point] = color
        pixel_point += 1
      end
    end

    file.close

    { width: width, height: height, pixels: pixels }
  end

  # Функция отрисовки изображения
  def draw
    Gosu.draw_rect(0, 0, width, height, Gosu::Color::WHITE) # Фон

    @image_data[:pixels].each_with_index do |color, index|
      x = index % @image_data[:width]
      y = index / @image_data[:width]

      Gosu.draw_rect(x * @factor, y * @factor, @factor, @factor, color)
    end
  end
end

# Запуск программы с аргументами
if ARGV.size < 1
  puts "Usage: ruby zpif_renderer.rb <path_to_zpif> [factor]"
  exit 1
end

file_path = ARGV[0]
factor = ARGV[1] ? ARGV[1].to_i : 1

ZPIFRenderer.new(file_path, factor).show
