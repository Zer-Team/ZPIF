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
///                     mono  6.12.0.206                     ///
///                     dotnet-sdk 9.0.1                     ///
///                        SFML 2.6.2                        ///
///                            C#                            ///
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
///                        ID: HM0100                        ///
///                     Date: 2025-02-05                     ///
///                     Author: Zer Team                     ///
////////////////////////////////////////////////////////////////

using System;
using System.IO;
using SFML.Graphics;
using SFML.System;
using SFML.Window;

namespace RenderZPIF
{
    // Класс для размещения методов
    public class Function
    {
        // Функция для чтения unsigned short из файла в формате big-endian
        public static ushort ConvertBEInShort(byte[] bytes)
        {
            return (ushort)((bytes[0] << 8) | bytes[1]);
        }

        // Функция для чтения unsigned int из файла в формате big-endian
        public static uint ConvertBEInInt(byte[] bytes)
        {
            return ((uint)bytes[1] << 24) |
                   ((uint)bytes[2] << 16) |
                   ((uint)bytes[3] << 8)  |
                   (uint)bytes[4];
        }

        // Функция для рендеринга изображения
        public static void RenderImage(uint width, uint height, ushort factor, byte[] pixelData)
        {
            // Размеры окна с учетом масштаба
            int scaledWidth = (int)(width * factor);
            int scaledHeight = (int)(height * factor);

            // Создание окна с размерами, соответствующими масштабу
            RenderWindow window = new RenderWindow(new VideoMode((uint)scaledWidth, (uint)scaledHeight), "ZPIF Renderer");

            // Обработчик закрытия окна
            window.Closed += (sender, e) => window.Close();

            // Рендеринг пикселей
            while (window.IsOpen)
            {
                window.DispatchEvents(); // Обработка событий

                window.Clear();

                for (int y = 0; y < height; y++)
                {
                    for (int x = 0; x < width; x++)
                    {
                        // Индекс пикселя в данных
                        int index = (int)((y * width + x) * 4);

                        // Извлечение цвета пикселя
                        byte r = pixelData[index];
                        byte g = pixelData[index + 1];
                        byte b = pixelData[index + 2];
                        byte a = pixelData[index + 3];

                        // Создание цвета для пикселя
                        Color color = new Color(r, g, b, a);

                        // Увеличение каждого пикселя в factor раз
                        RectangleShape pixel = new RectangleShape(new Vector2f(factor, factor));
                        pixel.FillColor = color;
                        pixel.Position = new Vector2f(x * factor, y * factor);

                        // Отрисовка
                        window.Draw(pixel);
                    }
                }

                window.Display();

                // Проверка события (например, закрытие окна)
                window.DispatchEvents();
            }
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("\u001B[1;33mUsage: main <path_to_zpif_file> [factor]\u001B[0m");
                return;
            }

            string filePath = args[0];          // Путь к файлу
            ulong  imagePoint = 0;              // Позиция пикселя
            uint   imageWidth = 0,              // Ширина изображения
                   imageHeight = 0;             // Высота изображения
            ushort factor = args.Length > 1 ? (ushort)Int16.Parse(args[1]) : (ushort)1; 

            byte[] pixelData = new byte[0];

            try
            {
                using (FileStream fs = new FileStream(filePath, FileMode.Open, FileAccess.Read))
                {
                    byte[] buffer = new byte[6]; // Буфер для хранения чанков

                    // Чтения первого заголовка
                    fs.Read(buffer, 0, buffer.Length);

                    // Проверка первого чанка
                    if (!(buffer[0] == 0x89 &&
                          buffer[1] == 0x5A && // Z
                          buffer[2] == 0x50 && // P
                          buffer[3] == 0x49 && // I
                          buffer[4] == 0x46 && // F
                          buffer[5] == 0x0A))
                    {
                        Console.WriteLine($"\u001B[1;31mError 1: The file is damaged or the format is not supported.\u001B[0m");
                        return;
                    }

                    // Работа с чанками параметров
                    while ((fs.Read(buffer, 0, buffer.Length)) > 0)
                    {
                        if (buffer[0] == 0x00 &&
                            buffer[1] == 0x00 &&
                            buffer[2] == 0xFF &&
                            buffer[3] == 0xFF &&
                            buffer[4] == 0xFF &&
                            buffer[5] == 0xFF)
                            break; // Выход при чанке FF

                        if (buffer[0] == 0x77)      // w
                            imageWidth = Function.ConvertBEInInt(buffer);
                        else if (buffer[0] == 0x68) // h
                            imageHeight = Function.ConvertBEInInt(buffer);
                    }

                    // Чтение пикселей
                    pixelData = new byte[imageWidth * imageHeight * 4];

                    // Работа с чанками пикселей
                    while ((fs.Read(buffer, 0, buffer.Length)) > 0)
                    {
                        if (buffer[0] == 0x00 &&
                            buffer[1] == 0x00 &&
                            buffer[2] == 0x00 &&
                            buffer[3] == 0x00 &&
                            buffer[4] == 0x00 &&
                            buffer[5] == 0x00)
                            break; // Выход при чанке 00

                        ushort quantity = Function.ConvertBEInShort(buffer);
                        byte r = buffer[2], g = buffer[3], b = buffer[4], a = buffer[5];

                        while (quantity > 0)
                        {
                            int index = (int)(imagePoint * 4);

                            pixelData[index] = r;
                            pixelData[index + 1] = g;
                            pixelData[index + 2] = b;
                            pixelData[index + 3] = a;

                            quantity--;
                            imagePoint++;
                        }
                    }

                    // Рендер изображения
                    Function.RenderImage(imageWidth, imageHeight, factor, pixelData);
                }
            }
            // Обработка ошибок
            catch (Exception ex)
            {
                Console.WriteLine($"\u001B[1;31mError: {ex.Message}\u001B[0m");
                return;
            }
        }
    }
}
