#!/bin/sh

readonly x='/home/admin/test.zpif'

cd ./render/java

# Компиляция main.java
javac main.java

# Создание manifest.txt
echo "Main-Class: main" > manifest.txt
echo "" >> manifest.txt

# Сжатие в JAR
jar cvfm main.jar manifest.txt main.class ImagePanel.class

# Очистка терминала
clear

# Вывод путя к файлу
echo "${x}"

# Запуск
java -jar main.jar

# Удаление
rm main.class ImagePanel.class main.jar manifest.txt