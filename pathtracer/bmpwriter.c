//
// Created by kennu on 20/06/2024.
//
#include <stdio.h>

void saveBMP(const char* filename, int width, int height, char* imageData)
{
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Unable to create BMP file %s\n", filename);
        return;
    }

    // BMP file header (14 bytes)
    char bmpFileHeader[14] = {
            'B', 'M',             // Signature "BM"
            0, 0, 0, 0,           // File size (in bytes) - To be filled
            0, 0, 0, 0,           // Reserved
            54, 0, 0, 0           // Offset to start of image data (default is 54 bytes)
    };

    // BMP info header (40 bytes)
    char bmpInfoHeader[40] = {
            40, 0, 0, 0,          // Header size (40 bytes)
            width & 0xFF, width >> 8 & 0xFF, width >> 16 & 0xFF, width >> 24 & 0xFF, // Image width
            height & 0xFF, height >> 8 & 0xFF, height >> 16 & 0xFF, height >> 24 & 0xFF, // Image height
            1, 0,                 // Number of color planes (must be 1)
            24, 0,                // Bits per pixel (24-bit BMP has 3 bytes per pixel)
            0, 0, 0, 0,           // Compression method (0 = BI_RGB, no compression)
            0, 0, 0, 0,           // Image size (can be 0 for BI_RGB)
            0, 0, 0, 0,           // Horizontal resolution (pixels/meter, can be 0)
            0, 0, 0, 0,           // Vertical resolution (pixels/meter, can be 0)
            0, 0, 0, 0,           // Number of colors in the palette (0 = default 2^n)
            0, 0, 0, 0            // Number of important colors (can be 0)
    };

    // Calculate image size
    int imageSize = width * height * 3; // 3 bytes per pixel for 24-bit BMP

    // Update BMP file header with file size
    int fileSize = 54 + imageSize; // Header size + image data size
    bmpFileHeader[2] = fileSize & 0xFF;
    bmpFileHeader[3] = (fileSize >> 8) & 0xFF;
    bmpFileHeader[4] = (fileSize >> 16) & 0xFF;
    bmpFileHeader[5] = (fileSize >> 24) & 0xFF;

    // Write BMP headers
    fwrite(bmpFileHeader, sizeof(char), 14, file);
    fwrite(bmpInfoHeader, sizeof(char), 40, file);

    // Write image data (BGR format)
    fwrite(imageData, sizeof(char), imageSize, file);

    // Close file
    fclose(file);

    printf("BMP file saved: %s\n", filename);
}