#ifndef WREN_C_
#define WREN_C_

#include <stdint.h>
#include <stddef.h>

#define WREN_SWAP(T, a, b) do { T t = a; a = b; b = t; } while (0)
#define WREN_SIGN(T, x) ((T)((x) > 0) - (T)((x) < 0))
#define WREN_ABS(T, x) (WREN_SIGN(T, x)*(x))

void wrenFill(uint32_t *pixels, size_t width, size_t height, uint32_t color) {
    for (size_t i = 0; i < width*height; i++) {
        pixels[i] = color;
    }
}

void wrenFillRect(uint32_t *pixels, size_t pixelsWidth, size_t pixelsHeight, 
                  int x1, int y1, int w, int h, uint32_t color) {
    int x2 = x1 + WREN_SIGN(int, w)*(WREN_ABS(int, w) - 1);
    if (x1 > x2) WREN_SWAP(int, x1, x2);
    int y2 = y1 + WREN_SIGN(int, h)*(WREN_ABS(int, h) - 1);
    if (y1 > y2) WREN_SWAP(int, y1, y2);

    for (int y = y1; y <= y2; y++) {
        if (0 <= y && y < (int) pixelsHeight) {
            for (int x = x1; x <= x2; x++) {
                if (0 <= x && x < (int) pixelsWidth) {
                    pixels[y*pixelsWidth + x] = color;
                }
            }
        }
    }
}

void wrenFillCircle(uint32_t *pixels, size_t pixelsWidth, size_t pixelHeight,
                    int cx, int cy, int r, uint32_t color) {
    if (r == 0) return;

    int x1 = cx - r;
    int x2 = cx + r;
    if (x1 > x2) WREN_SWAP(int, x1, x2);

    int y1 = cy - r;
    int y2 = cy + r;
    if (y1 > y2) WREN_SWAP(int, y1, y2);

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

void wrenFillTriangle(uint32_t *pixels, size_t width, size_t height,
                      int x1, int y1,
                      int x2, int y2,
                      int x3, int y3,
                      uint32_t color) {
    if (y1 > y2) {
        WREN_SWAP(int, x1, x2);
        WREN_SWAP(int, y1, y2);
    }

    if (y2 > y3) {
        WREN_SWAP(int, x2, x3);
        WREN_SWAP(int, y2, y3);
    }
    
    if (y1 > y2) {
        WREN_SWAP(int, x1, x2);
        WREN_SWAP(int, y1, y2);
    }

    int dx12 = x2 - x1;
    int dy12 = y2 - y1;
    int dx13 = x3 - x1;
    int dy13 = y3 - y1;

    for (int y = y1; y <= y2; ++y) {
        if (0 <= y && (size_t) y < height) {
            int s1 = dy12 != 0 ? (y - y1)*dx12/dy12 + x1 : x1;
            int s2 = dy13 != 0 ? (y - y1)*dx13/dy13 + x1 : x1;
            if (s1 > s2) WREN_SWAP(int, s1, s2);
            for (int x = s1; x <= s2; ++x) {
                if (0 <= x && (size_t) x < width) {
                    pixels[y*width + x] = color;
                }
            }
        }
    }

    int dx32 = x2 - x3;
    int dy32 = y2 - y3;
    int dx31 = x1 - x3;
    int dy31 = y1 - y3;

    for (int y = y2; y <= y3; ++y) {
        if (0 <= y && (size_t) y < height) {
            int s1 = dy32 != 0 ? (y - y3)*dx32/dy32 + x3 : x3;
            int s2 = dy31 != 0 ? (y - y3)*dx31/dy31 + x3 : x3;
            if (s1 > s2) WREN_SWAP(int, s1, s2);
            for (int x = s1; x <= s2; ++x) {
                if (0 <= x && (size_t) x < width) {
                    pixels[y*width + x] = color;
                }
            }
        }
    }
}

#endif