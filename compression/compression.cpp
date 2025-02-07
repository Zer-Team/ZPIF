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
///                          C++ 20                          ///
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
///                        ID: HM0100                        ///
///                     Date: 2025-02-07                     ///
///                     Author: Zer Team                     ///
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstdint>

// Преобразование числа в big-endian
inline void convertNumberInBE(uint16_t value, std::vector<uint8_t> &buffer, uint8_t shift = 0)
{
    buffer[0 + shift] = value >> 8;
    buffer[1 + shift] = value & 0xFF;
}

void compress_rle(std::ifstream &inputFile, std::ofstream &outputFile)
{
    std::vector<u_int8_t> bufferInp(6), // Буфер для чтения
                          bufferOut(6); // Буфер для записи
    std::vector<uint8_t>  fileBuffer;   // Буфер для накопления данных перед записью
    uint16_t count = 1;

    inputFile.read(reinterpret_cast<char *>(bufferInp.data()), bufferInp.size());
    if (bufferInp != std::vector<uint8_t>{0x89, 'Z', 'P', 'I', 'F', 0x0A})
        return;

    fileBuffer.insert(fileBuffer.end(), bufferInp.begin(), bufferInp.end());

    while (inputFile.read(reinterpret_cast<char *>(bufferInp.data()), bufferInp.size()))
    {
        if (bufferInp == std::vector<uint8_t>{0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF})
            break;
        
        // if (bufferInp[0] == 0x74) // t
        // {
        //     std::time_t nowTime = std::time(nullptr);
        //     std::tm *localTime = std::localtime(&nowTime);

        //     convertNumberInBE(static_cast<uint16_t>(localTime->tm_year + 1900), bufferOut, 1);
        //     bufferOut[0] = 0x74;
        //     bufferOut[3] = static_cast<uint8_t>(localTime->tm_mon + 1);
        //     bufferOut[4] = static_cast<uint8_t>(localTime->tm_mday);
        //     bufferOut[5] = static_cast<uint8_t>(localTime->tm_hour);
        // }

        else
            bufferOut = bufferInp;

        fileBuffer.insert(fileBuffer.end(), bufferOut.begin(), bufferOut.end());
    }
    fileBuffer.insert(fileBuffer.end(), {0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF});

    if (inputFile.read(reinterpret_cast<char *>(bufferOut.data()), bufferOut.size()))
    {
        while (inputFile.read(reinterpret_cast<char *>(bufferInp.data()), bufferInp.size()))
        {
            if (bufferInp == std::vector<uint8_t>(6, 0x00))
                break;

            if (bufferInp == bufferOut && count < 65535)
                count++;
            else
            {
                if (count > 1)
                    convertNumberInBE(count, bufferOut);
                fileBuffer.insert(fileBuffer.end(), bufferOut.begin(), bufferOut.end());
                bufferOut = bufferInp;
                count = 1;
            }
        }
        if (count > 1)
            convertNumberInBE(count, bufferOut);
        fileBuffer.insert(fileBuffer.end(), bufferOut.begin(), bufferOut.end());
    }
    fileBuffer.insert(fileBuffer.end(), {0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    outputFile.write(reinterpret_cast<char *>(fileBuffer.data()), fileBuffer.size());

    std::cout << "\033[1;32mCompression complite\033[0m" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
        return 1;
    }

    std::ifstream input_file (argv[1], std::ios::binary);
    std::ofstream output_file(argv[2], std::ios::binary);

    if (!input_file || !output_file)
    {
        std::cerr << "Error opening file\n";
        return 1;
    }

    compress_rle(input_file, output_file);

    return 0;
}
