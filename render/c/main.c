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
///                       GCC   14.2.1                       ///
<<<<<<< HEAD
///                           GTK3                           ///
=======
///                        SFML 2.6.2                        ///
>>>>>>> b4fc632 (Add C and update Go)
///                           C 99                           ///
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
///                        ID: HM0100                        ///
///                     Date: 2025-02-03                     ///
///                     Author: Zer Team                     ///
////////////////////////////////////////////////////////////////


// Библиотеки
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <gtk/gtk.h>

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t point;
    uint8_t *pixels; // Array to store pixel data
} Image;

// Function to read unsigned short from file in big-endian format
uint16_t convertBEInShort(const uint8_t *bytes)
{
    return (bytes[0] << 8) | bytes[1];
}

// Function to read unsigned int from file in big-endian format
uint64_t convertBEInInt(const uint8_t *bytes)
{
    return ((bytes[1]) << 24) |
           ((bytes[2]) << 16) |
           ((bytes[3]) << 8)  |
           ((bytes[4]));
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    Image *img = (Image*) data;
    if (!img || !img->pixels)
    {
        return FALSE;
    }

    double scale_factor = gtk_widget_get_scale_factor(widget);
    int scaled_width = (int)(img->width * scale_factor);
    int scaled_height = (int)(img->height * scale_factor);
    int window_width = gtk_widget_get_allocated_width(widget);
    int window_height = gtk_widget_get_allocated_height(widget);
    
    double x_scale = (double)window_width / scaled_width;
    double y_scale = (double)window_height / scaled_height;
    
    cairo_scale(cr, x_scale, y_scale);


    cairo_surface_t *surface = cairo_image_surface_create_for_data(
        img->pixels, 
        CAIRO_FORMAT_RGB24, 
        img->width, 
        img->height, 
        img->width * 4 // stride
    );
    
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);
    cairo_surface_destroy(surface);

    return TRUE;
}

int main(int argc, char **argv)
{
    Image img = {0, 0, 0, NULL};
    uint16_t factor = 1;


    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <path_file> <factor>\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "rb");
    factor = atoi(argv[2]);

    if (!file)
    {
        fprintf(stderr, "Error: Failed to open input file: %s\n", argv[1]);
        return 1;
    }

    uint8_t buffer[6] = {0};

    // Reading the first chunk
    fread(buffer, 1, 6, file);
    if (!(buffer[0] == 0x89 && 
          buffer[1] == 0x5A && // Z
          buffer[2] == 0x50 && // P
          buffer[3] == 0x49 && // I
          buffer[4] == 0x46 && // F
          buffer[5] == 0x0A))
    {
        puts("\033[1;31mError 1: The file is damaged or the format is not supported.\033[0m");
        fclose(file);
        return -1;
    }
    
    // Parsing sizes from parameter chunks
    while (fread(buffer, 1, 6, file))
    {
        if (buffer[0] == 0x00 && 
            buffer[1] == 0x00 && 
            buffer[2] == 0xFF &&
            buffer[3] == 0xFF &&
            buffer[4] == 0xFF &&
            buffer[5] == 0xFF)
        {
            break; // End if structuring chunk is found
        }
        
        if (buffer[0] == 0x77)      // w
            img.width = convertBEInInt(buffer);
        else if (buffer[0] == 0x68) // h
            img.height = convertBEInInt(buffer);
    }

    img.pixels = (uint8_t *)malloc(img.width * img.height * 4);
    if (!img.pixels)
    {
        fprintf(stderr, "Error: Failed to allocate memory for pixel data\n");
        fclose(file);
        return 1;
    }
    uint8_t *pixel_ptr = img.pixels;


    while (fread(buffer, 1, 6, file))
    {
        if (buffer[0] == 0x00 && 
            buffer[1] == 0x00 && 
            buffer[2] == 0x00 &&
            buffer[3] == 0x00 &&
            buffer[4] == 0x00 &&
            buffer[5] == 0x00)
        {
            break; // End if structuring chunk is found
        }

        uint16_t quantity = convertBEInShort(buffer);

        uint8_t r = buffer[2], g = buffer[3], b = buffer[4], a = buffer[5];

        for (uint16_t i = 0; i < quantity; i++)
        {
            if (img.point >= img.width * img.height) {
                break;
            }
            *pixel_ptr++ = b;
            *pixel_ptr++ = g;
            *pixel_ptr++ = r;
            *pixel_ptr++ = a;
            img.point++;
        }
    }
        
    gtk_init(&argc, &argv);
    
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Image Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), img.width * factor, img.height * factor);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), &img);

    gtk_widget_show_all(window);
    gtk_main();
    
    if (img.pixels) free(img.pixels);
    fclose(file);

    return 0;
}