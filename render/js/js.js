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
///                 Chromium: 132.0.6834.160                 ///
///                            JS                            ///
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
///                        ID: HM0100                        ///
///                     Date: 2025-02-02                     ///
///                     Author: Zer Team                     ///
////////////////////////////////////////////////////////////////

// Функция для чтения unsigned short из файла в формате big-endian
function convertBEInShort(bytes)
{
    return ((bytes[0] & 0xff) << 8) | (bytes[1] & 0xff);
}

// Функция для чтения unsigned int из файла в формате big-endian
function convertBEInInt(bytes)
{
    return ((bytes[1] & 0xff) << 24) |
           ((bytes[2] & 0xff) << 16) |
           ((bytes[3] & 0xff) << 8) |
           (bytes[4] & 0xff);
}

// Сравнения массивов
function arraysEqual(arr1, arr2)
{
    return arr1.length === arr2.length && arr1.every((v, i) => v === arr2[i]);
}

// Функция для изменения цвета одного пикселя
function setPixel(x, y, r, g, b, a = 255)
{
    const imageData = ctx.getImageData(x, y, 1, 1); // Получаем один пиксель
    const data = imageData.data;
  
    data[0] = r; // Красный
    data[1] = g; // Зелёный
    data[2] = b; // Синий
    data[3] = a; // Альфа (прозрачность)
  
    ctx.putImageData(imageData, x, y); // Обновления пикселя на canvas
}

const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");
const output = document.querySelector("#output");
const back = document.querySelector("img");

let imageWidth, imageHeight, imagePoint = 0;

// Обработка событий
document.addEventListener('DOMContentLoaded', () => {
    const fileInput = document.getElementById('fileInput');
    const dropZone = document.getElementById('dropZone');

    // Отмена действий по умолчанию для всех событий Drag & Drop
    ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => {
        dropZone.addEventListener(eventName, (e) => {
            e.preventDefault();
            e.stopPropagation();
        });
    });

    // Визуальная индикация перетаскивания
    dropZone.addEventListener('dragover', () => {
        dropZone.classList.add('dragover');
    });
    dropZone.addEventListener('dragleave', () => {
        dropZone.classList.remove('dragover');
    });

    // Обработка события "drop"
    dropZone.addEventListener('drop', (e) => {
        dropZone.classList.remove('dragover');
        const dt = e.dataTransfer;
        if (dt && dt.files && dt.files.length > 0) {
            const file = dt.files[0];
            console.log('File moved:', file.name);
            handleFile(file);
        }
    });

    // По клику по dropZone открывается диалог выбора файла
    dropZone.addEventListener('click', () => {
        fileInput.click();
    });

    // Обработка выбора файла через input
    fileInput.addEventListener('change', (e) => {
        if (e.target.files && e.target.files.length > 0) {
            const file = e.target.files[0];
            console.log('File selected:', file.name);
            handleFile(file);
        }
    });

    // Функция для чтения файла.
    function handleFile(file) {
        imagePoint = 0;

        output.innerHTML = "Render..."

        const reader = new FileReader();
        reader.onload = (event) => {
            const buffer = event.target.result;
            const dv = new DataView(buffer);
            const totalBytes = buffer.byteLength;
            let offset = 6;

            if (dv.getUint8(0) != 0x89 || 
                dv.getUint8(1) != 0x5A || 
                dv.getUint8(2) != 0x50 || 
                dv.getUint8(3) != 0x49 || 
                dv.getUint8(4) != 0x46 || 
                dv.getUint8(5) != 0x0A)
            {
                console.error("Error 1: The file is damaged or the format is not supported.");
                alert("Error 1: The file is damaged or the format is not supported.");
                output.innerHTML = ''; // Очистка
                return 1;
            }

            // Парсинг параметров
            while(offset < totalBytes)
            {
                let chunk = [];
                for (let i = 0; i < 6 && (offset + i) < totalBytes; i++)
                {
                    const byte = dv.getUint8(offset + i);
                    chunk.push(byte);
                }
                if (chunk[0] == 0x77) // w
                    imageWidth = convertBEInInt(chunk);
                else if (chunk[0] == 0x68) // h
                    imageHeight = convertBEInInt(chunk);
                else if (arraysEqual(chunk, [0x00, 0x00, 0xff, 0xff, 0xff, 0xff]))
                    break
                offset += 6
            }

            // Настройка canvas
            ctx.canvas.width  = imageWidth;
            ctx.canvas.height = imageHeight;
            // Вывод размеров
            output.innerHTML = `${file.name} ${imageWidth}x${imageHeight}`

            // Рендер
            while(offset < totalBytes)
                {
                    let chunk = [];
                    let quantity = 0;
                    for (let i = 0; i < 6 && (offset + i) < totalBytes; i++)
                    {
                        const byte = dv.getUint8(offset + i);
                        chunk.push(byte);
                    }
                    if (arraysEqual(chunk, [0x00, 0x00, 0x00, 0x00, 0x00, 0x00]))
                    {
                        console.log("%cRender complete.", "color: rgb(0, 255, 0);");
                        break;
                    }

                    quantity = convertBEInShort(chunk);

                    for (let i = 0; i < quantity; i++)
                    {
                        const r = dv.getUint8(offset + 2);
                        const g = dv.getUint8(offset + 3);
                        const b = dv.getUint8(offset + 4);
                        const a = dv.getUint8(offset + 5);
                        setPixel((imagePoint % imageWidth), (imagePoint / imageWidth), r, g, b, a);
                        imagePoint++;
                    }

                    offset += 6
                }
            };

        // Ошибка
        reader.onerror = (e) => {
            console.error("Error 2: The file is damaged or the format is not supported.", e);
            alert("Error 2: The file is damaged or the format is not supported.");
        };

        reader.readAsArrayBuffer(file);
    }
});
