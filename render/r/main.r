#   MIT License

#   Copyright (c) 2025 Zakhar Shakhanov

#   Permission is hereby granted, free of charge, to any person obtaining a copy  
#   of this software and associated documentation files (the "Software"), to deal  
#   in the Software without restriction, including without limitation the rights  
#   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell  
#   copies of the Software, and to permit persons to whom the Software is  
#   furnished to do so, subject to the following conditions:

#   The above copyright notice and this permission notice shall be included in all  
#   copies or substantial portions of the Software.

#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,  
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  
#   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  
#   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  
#   SOFTWARE.


#///////////////////////////////////////////////////////////////
#//                      Rscript  4.4.2                      ///
#//                            R                             ///
#///////////////////////////////////////////////////////////////

#///////////////////////////////////////////////////////////////
#//                        ID: HM0100                        ///
#//                     Date: 2025-02-08                     ///
#//                     Author: Zer Team                     ///
#///////////////////////////////////////////////////////////////

library(grid)

# Функция для чтения big-endian u16
convertBEInShort <- function(bytes) {
  as.integer(bytes[1]) * 256 + as.integer(bytes[2])
}

# Функция для чтения big-endian u32
convertBEInInt <- function(bytes) {
  as.integer(bytes[2]) * 16777216 + as.integer(bytes[3]) * 65536 + as.integer(bytes[4]) * 256 + as.integer(bytes[5])
}

# Функция для загрузки ZPIF
load_zpif <- function(filename) {
  if (is.null(filename) || filename == "") {
    stop("Ошибка: Путь к файлу не задан.")
  }
  if (!file.exists(filename)) {
    stop("Ошибка: Файл не найден.")
  }
  
  con <- file(filename, "rb")
  on.exit(close(con))
  
  # Чтение заголовка
  header <- readBin(con, what = "raw", size = 1, n = 6)
  if (!identical(header, as.raw(c(0x89, 0x5A, 0x50, 0x49, 0x46, 0x0A)))) {
    stop("Ошибка: Файл поврежден или формат не поддерживается.")
  }
  
  width <- height <- 0
  
  # Чтение параметров
  repeat {
    chunk <- readBin(con, what = "raw", size = 1, n = 6)
    if (length(chunk) < 6 || identical(chunk, as.raw(c(0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF)))) break
    
    if (chunk[1] == as.raw(0x77)) {  # 'w'
      width <- convertBEInInt(chunk)
    } else if (chunk[1] == as.raw(0x68)) {  # 'h'
      height <- convertBEInInt(chunk)
    }
  }
  
  if (width == 0 || height == 0) stop("Ошибка: Некорректные размеры изображения.")
  
  image <- array(0, dim = c(height, width, 3))
  
  point <- 1
  repeat {
    chunk <- readBin(con, what = "raw", size = 1, n = 6)
    if (length(chunk) < 6 || identical(chunk, as.raw(rep(0x00, 6)))) break
    
    quantity <- convertBEInShort(chunk)
    r <- as.integer(chunk[3]) / 255
    g <- as.integer(chunk[4]) / 255
    b <- as.integer(chunk[5]) / 255
    
    for (i in 1:quantity) {
      y <- ((point - 1) %/% width) + 1
      x <- ((point - 1) %% width) + 1
      if (y <= height && x <= width) {
        image[y, x, ] <- c(r, g, b)
      }
      point <- point + 1
    }
  }
  
  return(image)
}

# Главная функция
render_zpif <- function() {
  args <- commandArgs(trailingOnly = TRUE)
  if (length(args) == 0) {
    stop("Usage: Rscript <path_to_zpif_file>")
  }
  filename <- args[1]
  
  img <- tryCatch({
    load_zpif(filename)
  }, error = function(e) {
    cat("Error load image:", e$message, "\n")
    return(NULL)
  })
  
  if (!is.null(img)) {
    grid.raster(img, interpolate = FALSE)
  }
}

render_zpif()
