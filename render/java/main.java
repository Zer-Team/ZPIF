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
///                       javac 23.0.2                       ///
///                      openjdk 23.0.2                      ///
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
///                        ID: HM0100                        ///
///                     Date: 2025-02-02                     ///
///                     Author: Zer Team                     ///
////////////////////////////////////////////////////////////////

// Библиотеки
import javax.swing.JFrame;
import javax.swing.JPanel;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Dimension;
import java.awt.image.BufferedImage;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Scanner;

public class main {
    // Функция для чтения unsigned short из файла в формате big-endian
    public static int convertBEInShort(byte[] bytes) {
        return ((bytes[0] & 0xFF) << 8) | (bytes[1] & 0xFF);
    }

    // Функция для чтения unsigned int из файла в формате big-endian
    public static long convertBEInInt(byte[] bytes) {
        return ((long) (bytes[1] & 0xFF) << 24) |
               ((long) (bytes[2] & 0xFF) << 16) |
               ((long) (bytes[3] & 0xFF) << 8)  |
               ((long) (bytes[4] & 0xFF))       ;
    }

    public static void main(String[] args) {
        // Размеры изображения и point (объявления)
        long imageWidth = 0, imageHeight = 0, imagePoint = 0;
        // Создания объекта Scanner для чтения ввода от пользователя
        Scanner scanner = new Scanner(System.in);

        // Запрос у пользователя путь к файлу
        System.out.print("Enter the path to the file: ");
        String filePath = scanner.nextLine();

        // Запрос у пользователя factor`a
        System.out.print("Enter the scale factor (integer): ");
        String factorStr = scanner.nextLine();
        int factor = Integer.parseInt(factorStr);

        // Создание окна
        JFrame frame = new JFrame("Image Viewer");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        // Панель для отрисовки изображения
        BufferedImage image = null;
        ImagePanel imagePanel = new ImagePanel(image, factor);


        // Открытие файл в бинарном режиме для чтения
        try (FileInputStream fis = new FileInputStream(filePath)) {
            // Буфер для хранения чанков
            byte[] buffer = new byte[6];
            int bytesRead;

            bytesRead = fis.read(buffer);

            if (bytesRead != 6 || buffer[0] == 0x89 && buffer[1] == 0x5A && buffer[2] == 0x50 && buffer[3] == 0x49 && buffer[4] == 0x46 && buffer[5] == 0x0A) {
                System.out.println("Error 1: The file is damaged or the format is not supported.");
                System.exit(1); // Завершение программы
            }

            // Обработка чанков параметров
            while (fis.read(buffer) != -1) {
                if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == (byte)0xFF && buffer[3] == (byte)0xFF && buffer[4] == (byte)0xFF && buffer[5] == (byte)0xFF)
                    break;
                else if (buffer[0] == 0x77) // w
                    imageWidth = convertBEInInt(buffer);

                else if (buffer[0] == 0x68) // h
                    imageHeight = convertBEInInt(buffer);
            }
            
            image = new BufferedImage((int) imageWidth * factor, (int) imageHeight * factor, BufferedImage.TYPE_INT_ARGB);
            imagePanel.setImage(image);
            frame.add(imagePanel);
            frame.setSize((int) imageWidth * factor, (int) imageHeight * factor);
            frame.setLocationRelativeTo(null);
            frame.setVisible(true);

            // Обработка чанков пикселей
            while (fis.read(buffer) != -1) {
                int r = 0, g = 0, b = 0, a = 0;
                long quantity = 0;

                if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0x00 && buffer[3] == 0x00 && buffer[4] == 0x00 && buffer[5] == 0x00)
                    break;

                quantity = convertBEInShort(buffer);

                for (int i = 0; i < quantity; i++) {
                    r = 0xFF & buffer[2];
                    g = 0xFF & buffer[3];
                    b = 0xFF & buffer[4];
                    a = 0xFF & buffer[5];

                    int pixelColor = (a << 24) | (r << 16) | (g << 8) | b; // Собираем RGBA значение
                    int x = (int) (imagePoint % imageWidth);
                    int y = (int) (imagePoint / imageWidth);
                    for(int fx = 0; fx < factor; fx++)
                    {
                      for(int fy = 0; fy < factor; fy++)
                      {
                        image.setRGB(x * factor + fx, y * factor + fy, pixelColor);
                      }
                    }

                    imagePoint++;
                }
                imagePanel.repaint();
            }


        } catch (IOException e) {
            // Обработка ошибок
            System.err.println("Error opening file: " + e.getMessage());
        } finally {
            // Закрытие Scanner
            scanner.close();
        }
    }
}

class ImagePanel extends JPanel {
    private BufferedImage image;
    private int factor;
    public ImagePanel(BufferedImage image, int factor) {
        this.image = image;
        this.factor = factor;
    }
    public void setImage(BufferedImage image)
    {
        this.image = image;
    }
    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        if (image != null) {
             g.drawImage(image, 0, 0, this);
        }
    }
}

////////////////////////////////////////////////////////////////
///   На Java пишут разработчики на Android и те кто не ищут ///
/// лёгких путей.                                            ///
///   На Go пишут садисты.                                   ///
///                                                          ///
///   ZZakharC                                               ///
////////////////////////////////////////////////////////////////