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
///                       GCC   14.2.1                       ///
///                        SFML 2.6.2                        ///
///                         C++   20                         ///
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
///                        ID: HM0100                        ///
///                     Date: 2025-02-02                     ///
///                     Author: Zer Team                     ///
////////////////////////////////////////////////////////////////

// Библиотеки
#include <iostream>
#include <string>
#include <SFML/Graphics.hpp>

// Собственные библиотеки
#include "include/classes.hpp"
#include "include/render.hpp"
#include "include/parser.hpp"

// Добавления в область видимости
using std::cout;
using std::cerr;
using std::endl;

// Главная функция
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
<<<<<<< HEAD
        cerr << "\033[1;33mUsage: " << argv[0] << " <file_path>\033[0m" << endl;
=======
        cerr << "\033[33mUsage: " << argv[0] << " <file_path>\033[0m" << endl;
>>>>>>> b4fc632 (Add C and update Go)
        return 1;
    }
       
    // Переменные и тд
    std::string filepath{argv[1]};
    Image img; // Данные изображения

    // Парсинг параметров из файла
    if (parserParams(img, filepath) < 0) return 1;

    // Рендер
    if (render(filepath, img, filepath.data())) return 1;

    return 0;
}