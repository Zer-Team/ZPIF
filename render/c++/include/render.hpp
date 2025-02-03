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
#ifndef _RENDER_HPP_
#define _RENDER_HPP_

// Библиотеки
#include <fstream>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "./classes.hpp"
#include "./parser.hpp"

// Функция рендера
short render(const std::string &filePath, Image &img, char *windowName)
{
    // Объявление
    std::string str;
    std::ifstream file{filePath, std::ios::binary};
    u_char factor{1};
    std::vector<u_int8_t> buffer(6);
    char Error{0};

    if (img.width < 10 && img.height < 10)
        factor = 100; // Коэффициент уменьшения
    else if (img.width < 20 && img.height < 20)
        factor = 50; // Коэффициент уменьшения
    else if (img.width <= 200 && img.height <= 200)
        factor = 3; // Коэффициент уменьшения

    cout << "\033[33mStart rendering\033[0m" << endl;

    // Переменные для хранения состояния окна
    bool isFullscreen = false;                                           // Текущее состояние (false = оконный режим)
    sf::VideoMode windowedMode(img.width * factor, img.height * factor); // Размеры окна в оконном режиме
    sf::VideoMode fullscreenMode = sf::VideoMode::getDesktopMode();      // Размеры для полноэкранного режима

    // Создания окно
    sf::RenderWindow window(windowedMode, windowName, sf::Style::Close);

    // Создания изображение и текстуры
    sf::Image image;
    image.create(img.width, img.height, sf::Color::Transparent); // Изображение

    sf::Texture texture;
    texture.loadFromImage(image); // Загрузка изображение в текстуру

    // Создания спрайта для отображения текстуры
    sf::Sprite sprite(texture);

    file.seekg(img.renderStart);

    // Парсинг и заполнения пикселя
    while (file.read(reinterpret_cast<char *>(buffer.data()), buffer.size()))
    {
        // Парсинг пикселя
        Error = parserPixel(buffer, img);

        // Завершение рендера
        //   Ошибки
        if (Error < 0)
            return -1;
        //   Конец файла
        if (Error == 1)
            break; 

        // Проверка на равенства
        if (img.point > img.width * img.height)
            break;

        // Устанавливаем цвет пикселя
        sf::Color color(img.rgba[0], img.rgba[1], img.rgba[2], img.rgba[3]);

        // Обработка сжатия
        if (img.quantity > 0)
        {
            while (img.quantity > 0 && (img.point <= img.width * img.height))
            {
                image.setPixel((img.point % img.width), (img.point / img.width), color);
                img.quantity--;
                img.point++;
            }
        }
        // Проверка на на равенства
        if (img.point + img.quantity > img.width * img.height)
            cout << "\033[1;33mWARNING: Number of pixels exceeds available\033[0m" << endl;
    }

    // Обновления текстуру из изменённого изображения
    texture.update(image);

    // Фон
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("./background.jpg"))
    {
        std::cerr << "\033[1;31mError: Failed to load image!\033[0m" << std::endl;
        return 1;
    }
    
    // Нормализация фона
    sf::Sprite backgroundSprite{backgroundTexture};

    // Вычисляем масштаб для сохранения пропорций
    float *scale = new float{std::max(
        static_cast<float>(window.getSize().x) / backgroundTexture.getSize().x,
        static_cast<float>(window.getSize().y) / backgroundTexture.getSize().y
    )};

    // Устанавливаем масштаб спрайта
    backgroundSprite.setScale(*scale, *scale);

    // Центрируем текстуру
    backgroundSprite.setPosition(
        (static_cast<float>(window.getSize().x) - backgroundTexture.getSize().x * *scale) / 2.f,
        (static_cast<float>(window.getSize().y) - backgroundTexture.getSize().y * *scale) / 2.f
    );
    delete scale; scale = nullptr;

    // Основной цикл программы
    while (window.isOpen())
    {
        sf::Event event;

        // Обработка событий
        while (window.pollEvent(event))
        {
            // Закрытие окна
            if (event.type == sf::Event::Closed || event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
            {
                cout << "\033[33mExit\033[0m" << endl;
                window.close();
            }
            // Открытие на весь экран
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11)
            {
                // Закрываем текущее окно
                window.close();

                // Переключаем режим
                isFullscreen = !isFullscreen;

                // Создаем новое окно с нужным стилем
                if (isFullscreen)
                {
                    window.create(fullscreenMode, "Fullscreen Mode", sf::Style::Fullscreen);
                }
                else
                {
                    window.create(windowedMode, "Windowed Mode", sf::Style::Close);
                }
            }
            // Обработка изменения размера окна
            if (event.type == sf::Event::Resized)
            {
                // Новые размеры окна
                float newWidth = static_cast<float>(event.size.width);
                float newHeight = static_cast<float>(event.size.height);

                // Масштабируем спрайт, чтобы он соответствовал размеру окна
                sprite.setScale(newWidth / texture.getSize().x, newHeight / texture.getSize().y);
                // Вычисляем масштаб для сохранения пропорций
                scale = new float{std::max(
                    static_cast<float>(window.getSize().x) / backgroundTexture.getSize().x,
                    static_cast<float>(window.getSize().y) / backgroundTexture.getSize().y
                )};

                // Устанавливаем масштаб спрайта
                backgroundSprite.setScale(*scale, *scale);

                // Центрируем текстуру
                backgroundSprite.setPosition(
                    (static_cast<float>(window.getSize().x) - backgroundTexture.getSize().x * *scale) / 2.f,
                    (static_cast<float>(window.getSize().y) - backgroundTexture.getSize().y * *scale) / 2.f
                );
                delete scale; scale = nullptr;
            }
        }

        // Отрисовка
        window.clear(sf::Color::White);
        window.draw(backgroundSprite);
        window.draw(sprite);
        window.display();
    }

    return 0;
}

#endif