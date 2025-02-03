![Logo](images/ZPIF.png "Logo ZPIF")

**English (translation)**

# ZPIF File Format Description

ZPIF (ZerProjectImageFormat) is an open raster format for storing graphical information.

## File Structure

A ZPIF file consists of data blocks called **chunks**, each 6 bytes in size. Types of chunks:

1. **Header Chunk** — Always at the beginning of the file, used for identification.
2. **Parameter Chunks** — Define image properties (width, height, etc.).
3. **Structuring Chunks** — Mark the start of pixel data and the end of the file.
4. **Pixel Chunks** — Contain pixel color information.

## Header Chunk

The first chunk of the file always contains the signature `89 5A 50 49 46 0A` in hexadecimal (hex) format, or `�ZPIF` in text representation. This signature allows programs to identify the file as a ZPIF format file.

## Parameter Chunks (Image Properties)

Parameter chunks consist of two parts:

- **Name Block** (1 byte) — Contains an English letter representing the parameter name.
- **Data Block** (5 bytes) — Stores the parameter value. If the data is smaller than 5 bytes, the remaining bytes are filled with `00`.

**Example:** `77 00 00 00 02 00`

- `77` (letter `w`) indicates the width parameter.
- `00 00 00 02` — width value (`2`) in big-endian format.
- `00` — unused byte.

### Supported Parameters

#### Required Parameters (hex)

- `77 00 00 04 38 00` — Image width (1080 pixels). (`77` - `w`, `00 00 04 38` - `1080`, `00` - unused).
- `68 00 00 04 42 00` — Image height (1090 pixels). (`68` - `h`, `00 00 04 42` - `1090`, `00` - unused).

#### Optional Parameters (hex)

- `74 07 E9 02 01 0D` — File creation date. (`74` - `t`, `07 E9` - year, `02` - month, `01` - day, `0D` - hour).
- `75 02 00 00 00 00` — Time offset relative to UTC. (`75` - `u`, `02` - `+2`, `00 00 00 00` - unused).

## Structuring Chunks

Structuring chunks are recorded in the format `00 00 XX XX XX XX` and define the data structure:

- `00 00 FF FF FF FF` — Start of pixel data.
- `00 00 00 00 00 00` — End of file.

## Pixel Chunks

Pixel data is recorded in chunks consisting of two blocks:

- **First Block** (2 bytes) — Used for RLE compression (Run-Length Encoding). Stores a number from 0 to 65535 in big-endian format. **Important:** If both bytes are `00`, it is interpreted as a structuring chunk.
- **Second Block** (4 bytes) — Pixel color in RGBA format (red, green, blue, alpha channel).

Pixels are written from left to right, top to bottom.

## Chunk Recording Order

1. **Parameter Chunks** Recorded in arbitrary order.
2. **Start of Pixel Data Chunk** `00 00 FF FF FF FF`.
3. **Pixel Chunks** Pixel color data.
4. **End of File Chunk** `00 00 00 00 00 00`, marking the end of the file.

## ZPIF File Examples

### Example 1 (Binary Representation in Hex)

```
89 5A 50 49 46 0A 77 00 00 00 02 00 68 00 00 00 02 00 74 07 E9 02 01 0C 00 00 FF FF FF FF 00 04 FF 00 00 FF 00 00 00 00 00 00
```

### Example 2 (Text Representation, bytes that are not displayed are replaced by `N`)

```
�ZPIF
wNNNNNhNNNNNtN�NNNNN����NN�NN�NNNNNN
```

## Notes

1. After the `00 00 00 00 00 00` (end of file) chunk, any data may follow, but this should be avoided.

<br>

**Русский (original)**

# Описание формата файлов ZPIF

ZPIF (ZerProjectImageFormat) — это свободный растровый формат для хранения графической информации.

## Структура файла

Файл ZPIF состоит из блоков данных, называемых **чанками**, каждый размером 6 байт. Виды чанков:

1. **Заголовочный чанк** — Всегда в начале файла нужен для идентификации.
2. **Чанки параметров** — Задают свойства изображения (ширина, высота и т.д.).
3. **Чанки структурирования** — Отмечают начало данных пикселей и конец файла.
4. **Чанки пикселей** — Содержат информацию о цвете пикселей.

## Заголовочный чанк

Первый чанк файла всегда содержит сигнатуру `89 5A 50 49 46 0A` в шестнадцатеричном виде (hex), или `�ZPIF` в текстовом представлении. Эта сигнатура позволяет программам идентифицировать, что это файл формата ZPIF.

## Чанки параметров (свойства изображения)

Чанки параметров состоят из двух частей:

- **Блок названия** (1 байт) — Содержит английскую букву, обозначающую названия параметра.
- **Блок данных** (5 байт) — данные параметра. Если данных меньше 5 байт, оставшиеся заполняются `00`.

**Пример:** `77 00 00 00 02 00`

- `77` (буква `w`) указывает, что это параметр ширины.
- `00 00 00 02` — значение ширины (`2`) в формате big-endian.
- `00` — неиспользуемый байт.

### Поддерживаемые параметры

#### Обязательные параметры (hex)

- `77 00 00 04 38 00` — Ширина изображения (1080 пикселей). (`77` - `w`, `00 00 04 38` - `1080`, `00` - не используется).
- `68 00 00 04 42 00` — Высота изображения (1090 пикселей). (`68` - `h`, `00 00 04 42` - `1090`, `00` - не используется).

#### Необязательные параметры (hex)

- `74 07 E9 02 01 0D` — Дата создания файла. (`74` - `t`, `07 E9` - год, `02` - месяц, `01` - день, `0D` - час).
- `75 02 00 00 00 00` — Смещение по времени относительно UTC. (`75` - `u`, `02` - `+2`, `00 00 00 00` - не используются).

## Чанки структурирования

Чанки структурирования записываются в формате `00 00 XX XX XX XX` и определяют структуру данных:

- `00 00 FF FF FF FF` — Начало данных пикселей.
- `00 00 00 00 00 00` — Конец файла.

## Чанки пикселей

Пиксельные данные записываются в чанках из двух блоков:

- **Первый блок** (2 байта) — Используются для RLE-сжатия (Run-Length Encoding). Хранят число от 0 до 65535 в формате big-endian. **Важно:** Если оба байта равны `00`, это интерпретируется как чанк структурирования.
- **Второй блок** (4 байта) — цвет пикселя в формате RGBA (красный, зеленый, синий, альфа-канал).

Пиксели записываются слева направо, сверху вниз.

## Порядок записи чанков

1. **Чанки параметров** Записываются в произвольном порядке.
2. **Чанк начала данных пикселей**  `00 00 FF FF FF FF`.
3. **Чанки пикселей** Данные о цвете пикселей.
4. **Чанк конца файла** `00 00 00 00 00 00`,  обозначает конец файла.

## Примеры файлов ZPIF

### Пример 1 (бинарное представление в hex)

```
89 5A 50 49 46 0A 77 00 00 00 02 00 68 00 00 00 02 00 74 07 E9 02 01 0C 00 00 FF FF FF FF 00 04 FF 00 00 FF 00 00 00 00 00 00
```

### Пример 2 (текстовое представление, байты которые не отображаются заменены на `N`)

```
�ZPIF
wNNNNNhNNNNNtN�NNNNN����NN�NN�NNNNNN
```

## Примечания

1. После чанка `00 00 00 00 00 00` (конца файла) могут быть любые данные, но этого следует избегать.