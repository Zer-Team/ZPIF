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
///                        GCC 14.2.1                        ///
///                        SFML 2.6.2                        ///
///                          C++ 20                          ///
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
///                        ID: HM0100                        ///
///                     Date: 2025-02-06                     ///
///                     Author: Zer Team                     ///
////////////////////////////////////////////////////////////////

// Библиотеки
#include <iostream>
#include <string>
#include <array>
#include <fstream>
#include <cstdint>
#include <SFML/Graphics.hpp>

// Добавления в область видимости
using std::cout;
using std::cerr;
using std::endl;

// Функция для чтения unsigned short из файла в формате big-endian
inline uint16_t convertBEInShort(const std::array<uint8_t, 6> &bytes)
{
    return static_cast<u_int>(bytes[0] << 8) | static_cast<u_int>(bytes[1]);
}

// Функция для чтения unsigned int из файла в формате big-endian
inline uint32_t convertBEInInt(const std::array<uint8_t, 6> &bytes)
{
    return (static_cast<uint32_t>(bytes[1]) << 24) | // Не с 0 потому что 0 хранит имя параметра 
           (static_cast<uint32_t>(bytes[2]) << 16) |
           (static_cast<uint32_t>(bytes[3]) << 8)  |
           (static_cast<uint32_t>(bytes[4]));
}

// Главная функция
int main(int argc, char *argv[])
{
    // Проверка на корректность аргументов
    if (argc < 2)
    {
        cerr << "\033[1;33mUsage: " << argv[0] << " <path_to_zpif_file> [factor]\033[0m" << endl;
        return 1;
    } 
    else if (argc > 2 && (std::stoi(argv[2]) < 0 || std::stoi(argv[2]) > 65535))
    {
        cerr << "\033[1;33mError: The second argument must support a number between 1 and 65535.\033[0m" << endl;
        return 1;
    }
    
    // Переменные и тд
    uint32_t               imageWidth  {0}, // Ширина изображения
                           imageHeight {0}; // Высота изображения
    uint16_t               factor      {static_cast<uint16_t>((argc > 2) ? std::stoi(argv[2]) : 1)}; // Фактор для увелечения
    uint64_t               pixelPoint  {0}; // Номер пикселя
    std::array<uint8_t, 6> buffer      {};  // Буфер для хранения чанка

    // Открытие файла
    std::ifstream inputFile(argv[1], std::ios::binary);
    
    // Проверка успешности открытия файла
    if (!inputFile.is_open())
    {
        cerr << "\033[1;31mError opening input file\033[0m" << std::endl;
        return 1;
    }

    // Чтение первого чанка
    inputFile.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

    // Проверка чанка заголовка
    if (buffer != std::array<u_int8_t, 6>{0x89, 'Z', 'P', 'I', 'F', 0x0A})
    {
        cerr << "\033[1;31mError 1: The file is damaged or the format is not supported.\033[0m" << std::endl;
        return -1;
    }

    // Парсинг параметров из файла (работа с чанками параметров)
    while (inputFile.read(reinterpret_cast<char *>(buffer.data()), buffer.size()))
    {
        // Прекращение чтения при достижении чанка структурирования FF
        if (buffer == std::array<uint8_t, 6>{0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF})
        {
            if (imageWidth <= 0 || imageHeight <= 0)
            {
                cerr << "\033[1;31mError 2: The file is damaged or the format is not supported.\033[0m" << std::endl;
                return 1;
            }
            break;
        }
        // Парсинг размеров изображения
        if (buffer[0] == 0x77)      // w
            imageWidth = convertBEInInt(buffer);

        else if (buffer[0] == 0x68) // h
            imageHeight = convertBEInInt(buffer);
    }
         
    // Создания окно
    sf::RenderWindow window(sf::VideoMode(imageWidth * factor, imageHeight * factor), "ZPIF Render C++", sf::Style::Close);

    // Создания изображение и текстуры
    sf::Image image;
    image.create(imageWidth, imageHeight, sf::Color::Transparent); // Изображение

    sf::Texture texture;
    texture.loadFromImage(image); // Загрузка изображение в текстуру

    // Создания спрайта для отображения текстуры
    sf::Sprite sprite(texture);

    // Фон
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("background.jpg"))
    {
        std::cerr << "\033[1;31mError: Failed to load image!\033[0m" << std::endl;
        return 1;
    }

    // Нормализация фона
    sf::Sprite backgroundSprite{backgroundTexture};

    // Вычисления масштаба для сохранения пропорций
    float scaleBackgroundSprite {std::max(
        static_cast<float>(window.getSize().x) / backgroundTexture.getSize().x,
        static_cast<float>(window.getSize().y) / backgroundTexture.getSize().y
    )};

    // Установкам масштаба спрайта
    backgroundSprite.setScale(scaleBackgroundSprite, scaleBackgroundSprite);

    // Централизация текстуру
    backgroundSprite.setPosition(
        (static_cast<float>(window.getSize().x) - backgroundTexture.getSize().x * scaleBackgroundSprite) / 2.f,
        (static_cast<float>(window.getSize().y) - backgroundTexture.getSize().y * scaleBackgroundSprite) / 2.f
    );

    // Парсинг пикселей из файла (работа с чанками пикселей)
    while (inputFile.read(reinterpret_cast<char *>(buffer.data()), buffer.size()))
    {
        if (buffer == std::array<uint8_t, 6>{0x00, 0x00, 0x00, 0x00, 0x00, 0x00})
            break; // Выход при достижении чанка структурирования 00

        uint16_t quantity {convertBEInShort(buffer)};

        sf::Color color(buffer[2], buffer[3], buffer[4], buffer[5]);

        for (uint16_t i = 0; i < quantity; ++i) {
            image.setPixel((pixelPoint % imageWidth), (pixelPoint / imageWidth), color);
            pixelPoint++;
        }
    }
         
    // Обновления текстуры изображения
    texture.update(image);
    
    // Растягивание текстуры
    sprite.setScale(imageWidth * factor / texture.getSize().x, imageHeight * factor / texture.getSize().y);

    // Основной цикл программы
    while (window.isOpen())
    {
        sf::Event event;

        // Обработка событий
        while (window.pollEvent(event))
        {
            // Закрытие окна
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Отрисовка
        window.clear(sf::Color::White);
        window.draw(backgroundSprite);
        window.draw(sprite);
        window.display();
    }

    // Закрытие файла
    inputFile.close();

    return 0;
}