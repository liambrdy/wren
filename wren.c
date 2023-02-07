#ifndef WREN_C_
#define WREN_C_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define returnDefer(value) do { result = (value); goto defer; } while (0)
#define WREN_SWAP(T, a, b) do { T t = a; a = b; b = t; } while (0)

void wrenFill(uint32_t *pixels, size_t width, size_t height, uint32_t color) {
    for (size_t i = 0; i < width*height; i++) {
        pixels[i] = color;
    }
}

int wrenSaveToPPMFile(uint32_t *pixels, size_t width, size_t height, const char *filePath) {
    int result = 0;
    FILE *f = fopen(filePath, "wb");
    if (f == NULL) returnDefer(errno);

    fprintf(f, "P6\n%zu %zu 255\n", width, height);
    if (ferror(f)) returnDefer(errno);

    for (size_t i = 0; i < width*height; i++) {
        uint32_t pixel = pixels[i];
        uint8_t bytes[3] = {
            (pixel>>(8*0))&0xFF,
            (pixel>>(8*1))&0xFF,
            (pixel>>(8*2))&0xFF
        };
        fwrite(bytes, sizeof(bytes), 1, f);
        if (ferror(f)) returnDefer(errno);
    }

defer:
    if (f) fclose(f);
    return result;
}

void wrenFillRect(uint32_t *pixels, size_t pixelsWidth, size_t pixelsHeight, int x0, int y0, size_t w, size_t h, uint32_t color) {
    for (int dy = 0; dy < (int) h; dy++) {
        int y = y0 + dy;
        if (0 <= y && y < (int) pixelsHeight) {
            for (int dx = 0; dx < (int) w; dx++) {
                int x = x0 + dx;
                if (0 <= x && x < (int) pixelsWidth) {
                    pixels[y*pixelsWidth + x] = color;
                }
            }
        }
    }
}

void wrenFillCircle(uint32_t *pixels, size_t pixelsWidth, size_t pixelHeight,
                    int cx, int cy, int r, uint32_t color) {
    int x1 = cx - r;
    int y1 = cy - r;
    int x2 = cx + r;
    int y2 = cy + r;
    for (int y = y1; y <= y2; y++) {
        if (0 <= y && y < (int) pixelHeight) {
            for (int x = x1; x <= x2; x++) {
                if (0 <= x && x < (int) pixelsWidth) {
                    int dx = x - cx;
                    int dy = y - cy;
                    if (dx*dx + dy*dy <= r*r) {
                        pixels[y*pixelsWidth + x] = color;
                    }
                }
            }
        }
    }
}

void wrenDrawLine(uint32_t *pixels, size_t pixelsWidth, size_t pixelsHeight, 
                  int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    if (dx != 0) {
        int c = y1 - dy*x1/dx;

        if (x1 > x2) WREN_SWAP(int, x1, x2);
        for (int x = x1; x <= x2; x++) {
            if (0 <= x && x < (int) pixelsWidth) {
                int sy1 = dy*x/dx + c;
                int sy2 = dy*(x + 1)/dx + c;
                if (sy1 > sy2) WREN_SWAP(int, sy1, sy2);
                for (int y = sy1; y <= sy2; y++) {
                    if (0 <= y && y < (int) pixelsHeight) {
                        pixels[y*pixelsWidth + x] = color;
                    }
                }
            }
        }
    } else {
        int x = x1;
        if (0 <= x && x < (int) pixelsWidth) {
            if (y1 > y2) WREN_SWAP(int, y1, y2);
            for (int y = y1; y <= y2; y++) {
                if (0 <= y && y < (int) pixelsHeight) {
                    pixels[y*pixelsWidth + x] = color;
                }
            }
        }
    }
}

#endif