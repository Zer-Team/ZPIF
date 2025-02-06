package com.zerteam.zpifrender;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.OpenableColumns;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class MainActivity extends Activity {
    private static final int FILE_PICKER_REQUEST_CODE = 1; // Код запроса для выбора файла
    private Bitmap image; // Объект изображения
    private ImageView imageView; // Виджет для отображения изображения
    private long imageWidth = 0, imageHeight = 0, imagePoint = 0; // Переменные для хранения размеров и текущей позиции пикселя
    private ProgressBar progressBar; // Индикатор загрузки
    private Button button; // Кнопка для выбора файла

    // Создание пула потоков для выполнения фоновых задач
    private ExecutorService executorService = Executors.newSingleThreadExecutor();
    private Handler handler = new Handler(Looper.getMainLooper()); // Обработчик для обновления UI

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Создание элементов UI
        imageView = new ImageView(this);
        button = new Button(this);
        button.setText("Select file");
        button.setOnClickListener(v -> openFileChooser());

        progressBar = new ProgressBar(this, null, android.R.attr.progressBarStyleLarge);
        progressBar.setVisibility(View.GONE); // По умолчанию скрываем индикатор загрузки

        // Создание основного контейнера
        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.addView(button);
        layout.addView(imageView);
        layout.addView(progressBar);

        setContentView(layout);
    }

    // Открытие проводника для выбора файла
    private void openFileChooser() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*"); // Разрешаем выбор любого файла
        startActivityForResult(intent, FILE_PICKER_REQUEST_CODE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == FILE_PICKER_REQUEST_CODE && resultCode == RESULT_OK && data != null) {
            Uri uri = data.getData();
            if (uri != null) {
                button.setEnabled(false);
                progressBar.setVisibility(View.VISIBLE);

                // Сбрасывание параметров перед загрузкой нового изображения
                imageWidth = 0;
                imageHeight = 0;
                imagePoint = 0;
                image = null;
                imageView.setImageBitmap(null);

                // Запуск фоновой загрузки файла
                executorService.execute(() -> {
                    try {
                        InputStream inputStream = getContentResolver().openInputStream(uri);
                        if (inputStream != null) {
                            loadImage(inputStream, 1);
                            inputStream.close();
                        }
                    } catch (IOException e) {
                        handler.post(() -> {
                            Toast.makeText(this, "Error opening input file: " + e.getMessage(), Toast.LENGTH_LONG).show();
                            button.setEnabled(true);
                            progressBar.setVisibility(View.GONE);
                        });
                    } finally {
                        handler.post(() -> {
                            button.setEnabled(true);
                            progressBar.setVisibility(View.GONE);
                        });
                    }
                });
            }
        }
    }

    // Загрузка изображения из входного потока
    private void loadImage(InputStream inputStream, int factor) throws IOException {
        byte[] buffer = new byte[6];
        
        // Проверка сигнатуры файла (ZPIF) (проверка чанка заголовка)
        if (inputStream.read(buffer) != 6 || buffer[0] != (byte) 0x89 || buffer[1] != 0x5A || buffer[2] != 0x50 ||
                buffer[3] != 0x49 || buffer[4] != 0x46 || buffer[5] != 0x0A) {
            throw new IOException("Error 1: The file is damaged or the format is not supported.");
        }

        // Парсинг параметров (работа с чанками параметров)
        while (inputStream.read(buffer) != -1) {
            if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == (byte) 0xFF &&
                    buffer[3] == (byte) 0xFF && buffer[4] == (byte) 0xFF && buffer[5] == (byte) 0xFF) {
                break;
            } else if (buffer[0] == 0x77) {
                imageWidth = convertBEInInt(buffer);
            } else if (buffer[0] == 0x68) {
                imageHeight = convertBEInInt(buffer);
            }
        }

        // Проверка корректности размеров изображения
        if (imageWidth <= 0 || imageHeight <= 0) {
            throw new IOException("Error 1: The file is damaged or the format is not supported.");
        }

        // Создание изображения
        try {
            image = Bitmap.createBitmap((int) imageWidth * factor, (int) imageHeight * factor, Bitmap.Config.ARGB_8888);
        } catch (IllegalArgumentException | OutOfMemoryError e) {
            throw new IOException("Error creating image: " + e.getMessage());
        }

        // Чтение данные пикселей (работа с чанками пикселей)
        while (inputStream.read(buffer) != -1) {
            int r = buffer[2] & 0xFF;
            int g = buffer[3] & 0xFF;
            int b = buffer[4] & 0xFF;
            int a = buffer[5] & 0xFF;
            long quantity = convertBEInShort(buffer);

            for (int i = 0; i < quantity; i++) {
                int x = (int) (imagePoint % imageWidth);
                int y = (int) (imagePoint / imageWidth);
                int pixelColor = Color.argb(a, r, g, b);
                
                // Заполнения пикселей с учетом фактора масштабирования
                for (int fx = 0; fx < factor; fx++) {
                    for (int fy = 0; fy < factor; fy++) {
                        if (x * factor + fx < image.getWidth() && y * factor + fy < image.getHeight()) {
                            image.setPixel(x * factor + fx, y * factor + fy, pixelColor);
                        }
                    }
                }
                imagePoint++;
            }
        }

        // Отображение изображения на UI-потоке
        handler.post(() -> imageView.setImageBitmap(image));
    }

    // Функция для чтения unsigned short в формате big-endian
    private int convertBEInShort(byte[] bytes) {
        return ((bytes[0] & 0xFF) << 8) | (bytes[1] & 0xFF);
    }

    // Функция для чтения unsigned int в формате big-endian
    private long convertBEInInt(byte[] bytes) {
        return ((long) (bytes[1] & 0xFF) << 24) |
                ((long) (bytes[2] & 0xFF) << 16) |
                ((long) (bytes[3] & 0xFF) << 8)  |
                ((long) (bytes[4] & 0xFF));
    }
}
