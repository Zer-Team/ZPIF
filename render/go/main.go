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
///             go version  go1.23.5 linux/amd64             ///
///                            GO                            ///
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
///                        ID: HM0100                        ///
///                     Date: 2025-02-04                     ///
///                     Author: Zer Team                     ///
////////////////////////////////////////////////////////////////

package main

import (
	"fmt"
	"image"
	"image/color"
	"image/png"
	"os"
)

// Функция для чтения unsigned short из файла в формате big-endian
func convertBEInShort(bytes []byte) uint16 {
	return uint16(bytes[0])<<8 | uint16(bytes[1])
}

// Функция для чтения unsigned int из файла в формате big-endian
func convertBEInInt(bytes []byte) uint32 {
	return uint32(bytes[1])<<24 | uint32(bytes[2])<<16 | uint32(bytes[3])<<8 | uint32(bytes[4])
}

func main() {
	args := os.Args          // (аналог char* argv[])
	var imageWidth  uint = 0 // Ширина картинки
	var imageHeight uint = 0 // Высота картинки
	var imagePoint  uint = 0 // Текущий пиксель

	if len(args) < 3 {
		fmt.Println("Usage:", args[0], "<path_to_zpif_file>", "<y/n>(Convert ZPIF to PNG)")
		return
	}

	// Открытие файл
	file, err := os.Open(args[1])
	if err != nil {
		fmt.Println("Error opening file:", err)
		return
	}
	defer file.Close()

	// Буфер для чанков
	buffer := make([]byte, 6)
	file.Read(buffer)

	// Проверка чанка с заголовком
	if buffer[0] != 0x89 || buffer[1] != 0x5A || buffer[2] != 0x50 || buffer[3] != 0x49 || buffer[4] != 0x46 || buffer[5] != 0x0A {
		fmt.Println("Error 1: The file is damaged or the format is not supported.")
		return
	}

	// Работа с чанками параметров
	for {
		_, err := file.Read(buffer)
		// Выход при EOF или другой ошибке
		if err != nil {
			break 
		}

		if buffer[0] == 0x77 {        // w
			imageWidth = uint(convertBEInInt(buffer))
		} else if buffer[0] == 0x68 { // h
			imageHeight = uint(convertBEInInt(buffer))
		} else if buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0xFF && buffer[3] == 0xFF && buffer[4] == 0xFF && buffer[5] == 0xFF {
			break // Выход при достижении чанка структурирования
		}
	}

	// Проверка на нулевые размеры
	if imageWidth == 0 || imageHeight == 0 {
		fmt.Println("Error: Invalid image dimensions", imageWidth, "x", imageHeight)
		return
	}

	// Создаем новое изображение
	img := image.NewRGBA(image.Rect(0, 0, int(imageWidth), int(imageHeight)))
	fmt.Println("Image dimensions:", imageWidth, "x", imageHeight)

	// Рендер
	for {
		quantity := uint16(0) // Количество пикселей подряд
		_, err := file.Read(buffer)
		if err != nil || (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0x00 && buffer[3] == 0x00 && buffer[4] == 0x00 && buffer[5] == 0x00) {
			break // Выход
		}

		quantity = uint16(convertBEInShort(buffer))

		for i := quantity; i > 0; i-- {
		x := imagePoint % uint(imageWidth)
		y := imagePoint / uint(imageWidth)

		// Проверка
		if x < uint(imageWidth) && y < uint(imageHeight) {
			img.Set(int(x), int(y), color.RGBA{
				R: buffer[2],
				G: buffer[3],
					B: buffer[4],
					A: buffer[5],
				})
			}

			imagePoint++
		}
	}

	// Вывод сообщения о завершении рендеринга
	fmt.Println("Image rendering completed.")

	// Конвертация ZPIF в PNG изображение
	if args[2] == "y" || args[2] == "Y" {
		outFile, err := os.Create("output.png")
		if err != nil {
			fmt.Println("Error creating output file:", err)
			return
		}
		defer outFile.Close()
		png.Encode(outFile, img)
		fmt.Println("Converted to PNG: output.png")
	}
}

////////////////////////////////////////////////////////////////
///   На Java пишут разработчики на Android и те кто не ищут ///
/// лёгких путей.                                            ///
///   На Go пишут садисты.                                   ///
///                                                          ///
///   ZZakharC                                               ///
////////////////////////////////////////////////////////////////