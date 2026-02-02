/*
    stb_image - v2.28 - public domain image loader

    This is a minimal header for stb_image.
    For the full version, download from: https://github.com/nothings/stb

    To use: Add STB_IMAGE_IMPLEMENTATION in ONE cpp file before including this.
*/

#ifndef STB_IMAGE_INCLUDE_STB_IMAGE_H
#define STB_IMAGE_INCLUDE_STB_IMAGE_H

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Image loading functions
unsigned char* stbi_load(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels);
void stbi_image_free(void* retval_from_stbi_load);

// Set to flip images vertically
void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);

#ifdef __cplusplus
}
#endif

#ifdef STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int stbi__vertically_flip_on_load = 0;

void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip) {
    stbi__vertically_flip_on_load = flag_true_if_should_flip;
}

// Simple BMP loader (supports 24-bit and 32-bit uncompressed BMPs)
static unsigned char* stbi_load_bmp(FILE* f, int* x, int* y, int* comp) {
    unsigned char header[54];
    if (fread(header, 1, 54, f) != 54) return NULL;
    
    if (header[0] != 'B' || header[1] != 'M') return NULL;
    
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int bitsPerPixel = *(short*)&header[28];
    int compression = *(int*)&header[30];
    
    if (compression != 0) return NULL; // Only uncompressed
    if (bitsPerPixel != 24 && bitsPerPixel != 32) return NULL;
    
    int channels = bitsPerPixel / 8;
    int rowSize = ((width * channels + 3) / 4) * 4; // Row padding to 4 bytes
    
    unsigned char* data = (unsigned char*)malloc(width * height * channels);
    if (!data) return NULL;
    
    int dataOffset = *(int*)&header[10];
    fseek(f, dataOffset, SEEK_SET);
    
    for (int row = 0; row < height; row++) {
        int destRow = stbi__vertically_flip_on_load ? (height - 1 - row) : row;
        unsigned char* dest = data + destRow * width * channels;
        
        for (int col = 0; col < width; col++) {
            unsigned char bgra[4];
            if (fread(bgra, 1, channels, f) != (size_t)channels) {
                free(data);
                return NULL;
            }
            // Convert BGR(A) to RGB(A)
            dest[col * channels + 0] = bgra[2]; // R
            dest[col * channels + 1] = bgra[1]; // G
            dest[col * channels + 2] = bgra[0]; // B
            if (channels == 4) {
                dest[col * channels + 3] = bgra[3]; // A
            }
        }
        
        // Skip padding
        int padding = rowSize - width * channels;
        if (padding > 0) fseek(f, padding, SEEK_CUR);
    }
    
    *x = width;
    *y = height;
    *comp = channels;
    return data;
}

// Simple TGA loader
static unsigned char* stbi_load_tga(FILE* f, int* x, int* y, int* comp) {
    unsigned char header[18];
    if (fread(header, 1, 18, f) != 18) return NULL;
    
    int width = header[12] | (header[13] << 8);
    int height = header[14] | (header[15] << 8);
    int bpp = header[16];
    int imageType = header[2];
    
    if (imageType != 2 && imageType != 3) return NULL; // Only uncompressed RGB/Grayscale
    if (bpp != 24 && bpp != 32 && bpp != 8) return NULL;
    
    int channels = bpp / 8;
    int size = width * height * channels;
    
    unsigned char* data = (unsigned char*)malloc(size);
    if (!data) return NULL;
    
    // Skip ID field
    if (header[0] > 0) fseek(f, header[0], SEEK_CUR);
    
    if (fread(data, 1, size, f) != (size_t)size) {
        free(data);
        return NULL;
    }
    
    // Convert BGR to RGB
    if (channels >= 3) {
        for (int i = 0; i < size; i += channels) {
            unsigned char temp = data[i];
            data[i] = data[i + 2];
            data[i + 2] = temp;
        }
    }
    
    // Flip if needed
    if (stbi__vertically_flip_on_load) {
        int rowSize = width * channels;
        unsigned char* temp = (unsigned char*)malloc(rowSize);
        for (int row = 0; row < height / 2; row++) {
            memcpy(temp, data + row * rowSize, rowSize);
            memcpy(data + row * rowSize, data + (height - 1 - row) * rowSize, rowSize);
            memcpy(data + (height - 1 - row) * rowSize, temp, rowSize);
        }
        free(temp);
    }
    
    *x = width;
    *y = height;
    *comp = channels;
    return data;
}

unsigned char* stbi_load(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    
    unsigned char* result = NULL;
    
    // Check file extension
    const char* ext = strrchr(filename, '.');
    if (ext) {
        if (_stricmp(ext, ".bmp") == 0) {
            result = stbi_load_bmp(f, x, y, channels_in_file);
        }
        else if (_stricmp(ext, ".tga") == 0) {
            result = stbi_load_tga(f, x, y, channels_in_file);
        }
    }
    
    fclose(f);
    
    // Note: For PNG/JPG support, use the full stb_image.h from https://github.com/nothings/stb
    
    return result;
}

void stbi_image_free(void* retval_from_stbi_load) {
    free(retval_from_stbi_load);
}

#endif // STB_IMAGE_IMPLEMENTATION

#endif // STB_IMAGE_INCLUDE_STB_IMAGE_H
