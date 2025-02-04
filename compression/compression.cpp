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
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <stdint.h>

#define NPOS std::string::npos

// Преобразование числа в big-endian
inline void convertNumberInBE(u_short value, std::vector<u_int8_t> &buffer, uint8_t shift = 0)
{
    buffer[0+shift] = value >> 8;
    buffer[1+shift] = value & 0xFF;
}

void compress_rle(std::ifstream &inputFile, std::ofstream &outputFile)
{
    std::vector<u_int8_t> bufferInp(6), // Буфер для чтения
                          bufferOut(6); // Буфер для записи
    unsigned short count = 1;

    // Проверка чанка заголовка
    inputFile.read(reinterpret_cast<char *>(bufferInp.data()), bufferInp.size());
    if (bufferInp !=  std::vector<u_int8_t>{0x89, 'Z', 'P', 'I', 'F', 0x0A})
        return;

    outputFile.write("\x89ZPIF\x0A", 6);

    // Работа с чанками параметров 
    while (inputFile.read(reinterpret_cast<char *>(bufferInp.data()), bufferInp.size()))
    {
        // Завершаем работы при нахождении чанка структурирования
        if (bufferInp == std::vector<u_int8_t>{0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF})
            break;
        
        if (bufferInp[0] == 0x74) // t
        {
            std::time_t nowTime = std::time(nullptr);             // Получения текущее время
            std::tm *localTime = std::localtime(&nowTime);        // Преобразования в локальное время

            convertNumberInBE(static_cast<u_short>(localTime->tm_year + 1900), bufferOut, 1); // Преобразования года
            
            bufferOut[0] = 0x74; // t
            bufferOut[3] = static_cast<u_int8_t>(localTime->tm_mon + 1);
            bufferOut[4] = static_cast<u_int8_t>(localTime->tm_mday);
            bufferOut[5] = static_cast<u_int8_t>(localTime->tm_hour);
        }
        else
            bufferOut = bufferInp;

        outputFile.write(reinterpret_cast<char *>(bufferOut.data()), bufferOut.size());
    }
    
    outputFile.write("\x00\x00\xFF\xFF\xFF\xFF", 6);

    // Работа с чанками пикселями
    if (inputFile.read(reinterpret_cast<char *>(bufferOut.data()), bufferOut.size()))
    {
        // Чтения остальных чанков
        while (inputFile.read(reinterpret_cast<char *>(bufferInp.data()), bufferInp.size()))
        {
            // Завершаем сжатие при встрече 6 нулевых байтов (конца файла)
            if (bufferInp == std::vector<u_int8_t>(6, 0x00))
                break;

            // Проверка данных из буфера out и буфера inp (одинаковые ли они) и не превышает ли count двух байтов
            if (bufferInp == bufferOut && count < 65535)
                count++;
            else
            {
                // Проверка нужна ли запись count в блок буфера
                if (count > 1)
                {
                    convertNumberInBE(count, bufferOut);
                }
                outputFile.write(reinterpret_cast<char *>(bufferOut.data()), bufferOut.size());
                bufferOut = bufferInp;
                count = 1;
            }
        }
        if (count > 1)
        {
            convertNumberInBE(count, bufferOut);
        }
        outputFile.write(reinterpret_cast<char *>(bufferOut.data()), bufferOut.size());
    }
    
    outputFile.write("\x00\x00\x00\x00\x00\x00", 6);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
        // return 1;
    }

    std::ifstream input_file(argv[1], std::ios::binary);
    std::ofstream output_file(argv[2], std::ios::binary);
    if (!input_file || !output_file)
    {
        std::cerr << "Error opening input file\n";
        return 1;
    }

    compress_rle(input_file, output_file);

    input_file.close();
    output_file.close();

    return 0;
}