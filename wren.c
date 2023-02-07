#ifndef WREN_C_
#define WREN_C_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef WRENDEF
#define WRENDEF static inline
#endif

#define WREN_SWAP(T, a, b) do { T t = a; a = b; b = t; } while (0)
#define WREN_SIGN(T, x) ((T)((x) > 0) - (T)((x) < 0))
#define WREN_ABS(T, x) (WREN_SIGN(T, x)*(x))

typedef struct {
    uint32_t *pixels;
    size_t width;
    size_t height;
    size_t stride;
} WrenCanvas;

#define WREN_CANVAS_NULL ((WrenCanvas) {0})
#define WREN_PIXEL(wc, x, y) (wc).pixels[(y)*(wc).stride + (x)]

WRENDEF WrenCanvas wrenMakeCanvas(uint32_t *pixels, size_t width, size_t height) {
    WrenCanvas wc = {
        .pixels = pixels,
        .width = width,
        .height = height,
        .stride = width
    };

    return wc;
}

WRENDEF bool wrenNormalizeRect(int x, int y, int w, int h,
                               size_t pixelsWidth, size_t pixelsHeight,
                               int *x1, int *x2, int *y1, int *y2) {
    *x1 = x;
    *y1 = y;

    *x2 = *x1 + WREN_SIGN(int, w)*(WREN_ABS(int, w) - 1);
    if (*x1 > *x2) WREN_SWAP(int, *x1, *x2);
    *y2 = *y1 + WREN_SIGN(int, h)*(WREN_ABS(int, h) - 1);
    if (*y1 > *y2) WREN_SWAP(int, *y1, *y2);

    if (*x1 >= (int) pixelsWidth) return false;
    if (*x2 < 0) return false;
    if (*y1 >= (int) pixelsHeight) return false;
    if (*y2 < 0) return false;

    if (*x1 < 0) *x1 = 0;
    if (*x2 >= (int) pixelsWidth) *x2 = (int) pixelsWidth - 1;
    if (*y1 < 0) *y1 = 0;
    if (*y2 >= (int) pixelsHeight) *y2 = (int) pixelsHeight - 1;

    return true;
}

WRENDEF WrenCanvas wrenSubcanvas(WrenCanvas wc, int x, int y, int w, int h) {
    int x1, x2, y1, y2;
    if (!wrenNormalizeRect(x, y, w, h, wc.width, wc.height, &x1, &x2, &y1, &y2)) return WREN_CANVAS_NULL;
    wc.pixels = &WREN_PIXEL(wc, x1, y1);
    wc.width = x2 - x1 + 1;
    wc.height = y2 - y1 + 1;
    return wc;
}

WRENDEF void wrenBlendColors(uint32_t *c1, uint32_t c2) {
    uint32_t r1 = ((*c1)>>(0*8))&0xFF;
    uint32_t g1 = ((*c1)>>(1*8))&0xFF;
    uint32_t b1 = ((*c1)>>(2*8))&0xFF;
    uint32_t a1 = ((*c1)>>(3*8))&0xFF;

    uint32_t r2 = (c2>>(0*8))&0xFF;
    uint32_t g2 = (c2>>(1*8))&0xFF;
    uint32_t b2 = (c2>>(2*8))&0xFF;
    uint32_t a2 = (c2>>(3*8))&0xFF;

    r1 = (r1*(255 - a2) + r2*a2)/255; if (r1 > 255) r1 = 255;
    g1 = (g1*(255 - a2) + g2*a2)/255; if (g1 > 255) g1 = 255;
    b1 = (b1*(255 - a2) + b2*a2)/255; if (b1 > 255) b1 = 255;

    *c1 = (r1<<(0*8)) | (g1<<(1*8)) | (b1<<(2*8)) | (a1<<(3*8));
}

WRENDEF void wrenFill(WrenCanvas wc, uint32_t color) {
    for (size_t y = 0; y < wc.height; y++) {
        for (size_t x = 0; x < wc.width; x++) {
            WREN_PIXEL(wc, x, y) = color;
        }
    }
}

void wrenFillRect(WrenCanvas wc, int x, int y, int w, int h, uint32_t color) {
    int x1, y1, x2, y2;
    if (!wrenNormalizeRect(x, y, w, h, wc.width, wc.height, &x1, &x2, &y1, &y2)) return;

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            wrenBlendColors(&WREN_PIXEL(wc, x, y), color);
        }
    }
}

void wrenFillCircle(WrenCanvas wc, int cx, int cy, int r, uint32_t color) {
    int x1, x2, y1, y2;
    int r1 = r + WREN_SIGN(int, r);
    if (!wrenNormalizeRect(cx - r1, cy - r1, 2*r1, 2*r1, wc.width, wc.height, &x1, &x2, &y1, &y2)) return;

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            int dx = x - cx;
            int dy = y - cy;
            if (dx*dx + dy*dy <= r*r) {
                wrenBlendColors(&WREN_PIXEL(wc, x, y), color);
            }
        }
    }
}

void wrenDrawLine(WrenCanvas wc, int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    if (dx != 0) {
        int c = y1 - dy*x1/dx;

        if (x1 > x2) WREN_SWAP(int, x1, x2);
        for (int x = x1; x <= x2; x++) {
            if (0 <= x && x < (int) wc.width) {
                int sy1 = dy*x/dx + c;
                int sy2 = dy*(x + 1)/dx + c;
                if (sy1 > sy2) WREN_SWAP(int, sy1, sy2);
                for (int y = sy1; y <= sy2; y++) {
                    if (0 <= y && y < (int) wc.height) {
                        wrenBlendColors(&WREN_PIXEL(wc, x, y), color);
                    }
                }
            }
        }
    } else {
        int x = x1;
        if (0 <= x && x < (int) wc.width) {
            if (y1 > y2) WREN_SWAP(int, y1, y2);
            for (int y = y1; y <= y2; y++) {
                if (0 <= y && y < (int) wc.height) {
                    wrenBlendColors(&WREN_PIXEL(wc, x, y), color);
                }
            }
        }
    }
}

void wrenFillTriangle(WrenCanvas wc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
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
        if (0 <= y && (size_t) y < wc.height) {
            int s1 = dy12 != 0 ? (y - y1)*dx12/dy12 + x1 : x1;
            int s2 = dy13 != 0 ? (y - y1)*dx13/dy13 + x1 : x1;
            if (s1 > s2) WREN_SWAP(int, s1, s2);
            for (int x = s1; x <= s2; ++x) {
                if (0 <= x && (size_t) x < wc.width) {
                    wrenBlendColors(&WREN_PIXEL(wc, x, y), color);
                }
            }
        }
    }

    int dx32 = x2 - x3;
    int dy32 = y2 - y3;
    int dx31 = x1 - x3;
    int dy31 = y1 - y3;

    for (int y = y2; y <= y3; ++y) {
        if (0 <= y && (size_t) y < wc.height) {
            int s1 = dy32 != 0 ? (y - y3)*dx32/dy32 + x3 : x3;
            int s2 = dy31 != 0 ? (y - y3)*dx31/dy31 + x3 : x3;
            if (s1 > s2) WREN_SWAP(int, s1, s2);
            for (int x = s1; x <= s2; ++x) {
                if (0 <= x && (size_t) x < wc.width) {
                    wrenBlendColors(&WREN_PIXEL(wc, x, y), color);
                }
            }
        }
    }
}

#endif