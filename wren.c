#ifndef WREN_C_
#define WREN_C_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef WRENDEF
#define WRENDEF static inline
#endif

#ifndef WREN_AA_RES
#define WREN_AA_RES 2
#endif

#define WREN_SWAP(T, a, b) do { T t = a; a = b; b = t; } while (0)
#define WREN_SIGN(T, x) ((T)((x) > 0) - (T)((x) < 0))
#define WREN_ABS(T, x) (WREN_SIGN(T, x)*(x))

typedef struct {
    size_t width, height;
    const char *glyphs;
} WrenFont;

#define DEFAULT_FONT_HEIGHT 5
#define DEFAULT_FONT_WIDTH 5
static char defaultFontGlyphs[128][DEFAULT_FONT_HEIGHT][DEFAULT_FONT_WIDTH] = {
    ['a'] = {
        {0, 1, 1, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 1, 1, 1, 0},
        {1, 0, 0, 1, 0},
        {0, 1, 1, 1, 0},
    },
    ['b'] = {
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 1, 1, 0, 0},
        {1, 0, 0, 1, 0},
        {1, 1, 1, 0, 0},
    },
    ['c'] = {
        {0, 1, 1, 0, 0},
        {1, 0, 0, 1, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 1, 0},
        {0, 1, 1, 0, 0},
    },
    ['d'] = {
        {0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0},
        {0, 1, 1, 1, 0},
        {1, 0, 0, 1, 0},
        {0, 1, 1, 1, 0},
    },
    ['e'] = {
        {0, 1, 1, 0, 0},
        {1, 0, 0, 1, 0},
        {1, 1, 1, 1, 0},
        {1, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
    },
    ['f'] = {
        {0, 0, 1, 1, 0},
        {0, 1, 0, 0, 0},
        {1, 1, 1, 1, 0},
        {0, 1, 0, 0, 0},
        {0, 1, 0, 0, 0},
    },
    ['p'] = {
        {1, 1, 1, 0, 0},
        {1, 0, 0, 1, 0},
        {1, 1, 1, 0, 0},
        {1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0},
    },
    ['o'] = {
        {0, 1, 1, 0, 0},
        {1, 0, 0, 1, 0},
        {1, 0, 0, 1, 0},
        {1, 0, 0, 1, 0},
        {0, 1, 1, 0, 0},
    },
};

static WrenFont defaultFont = {
    .glyphs = &defaultFontGlyphs[0][0][0],
    .width = DEFAULT_FONT_WIDTH,
    .height = DEFAULT_FONT_HEIGHT,
};

typedef struct {
    uint32_t *pixels;
    size_t width;
    size_t height;
    size_t stride;
} WrenCanvas;

#define WREN_CANVAS_NULL ((WrenCanvas) {0})
#define WREN_PIXEL(wc, x, y) (wc).pixels[(y)*(wc).stride + (x)]

#define WREN_RED(color)   (((color)&0x000000FF)>>(8*0))
#define WREN_GREEN(color) (((color)&0x0000FF00)>>(8*1))
#define WREN_BLUE(color)  (((color)&0x00FF0000)>>(8*2))
#define WREN_ALPHA(color) (((color)&0xFF000000)>>(8*3))
#define WREN_RGBA(r, g, b, a) ((((r)&0xFF)<<(8*0)) | (((g)&0xFF)<<(8*1)) | (((b)&0xFF)<<(8*2)) | (((a)&0xFF)<<(8*3)))

WRENDEF WrenCanvas wrenCanvas(uint32_t *pixels, size_t width, size_t height, size_t stride);
WRENDEF WrenCanvas wrenSubcanvas(WrenCanvas wc, int x, int y, int w, int h);
WRENDEF void wrenBlendColors(uint32_t *c1, uint32_t c2);
WRENDEF void wrenFill(WrenCanvas wc, uint32_t color);
WRENDEF void wrenRect(WrenCanvas wc, int x, int y, int w, int h, uint32_t color);
WRENDEF void wrenCircle(WrenCanvas wc, int cx, int cy, int r, uint32_t color);
WRENDEF void wrenLine(WrenCanvas wc, int x1, int y1, int x2, int y2, uint32_t color);
WRENDEF void wrenTriangle3(WrenCanvas wc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t c1, uint32_t c2, uint32_t c3);
WRENDEF void wrenTriangle(WrenCanvas wc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
WRENDEF void wrenText(WrenCanvas wc, const char *text, int x, int y, WrenFont font, size_t size, uint32_t color);

WRENDEF void wrenCopy(WrenCanvas src, WrenCanvas dst);

WRENDEF bool wrenNormalizeRect(int x, int y, int w, int h, size_t pixelsWidth, size_t pixelsHeight, int *x1, int *x2, int *y1, int *y2);

#endif

#ifdef WREN_IMPLEMENTATION

WRENDEF WrenCanvas wrenCanvas(uint32_t *pixels, size_t width, size_t height, size_t stride) {
    WrenCanvas wc = {
        .pixels = pixels,
        .width = width,
        .height = height,
        .stride = stride
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
    uint32_t r1 = WREN_RED(*c1);
    uint32_t g1 = WREN_GREEN(*c1);
    uint32_t b1 = WREN_BLUE(*c1);
    uint32_t a1 = WREN_ALPHA(*c1);

    uint32_t r2 = WREN_RED(c2);
    uint32_t g2 = WREN_GREEN(c2);
    uint32_t b2 = WREN_BLUE(c2);
    uint32_t a2 = WREN_ALPHA(c2);

    r1 = (r1*(255 - a2) + r2*a2)/255; if (r1 > 255) r1 = 255;
    g1 = (g1*(255 - a2) + g2*a2)/255; if (g1 > 255) g1 = 255;
    b1 = (b1*(255 - a2) + b2*a2)/255; if (b1 > 255) b1 = 255;

    *c1 = WREN_RGBA(r1, g1, b1, a1);
}

WRENDEF void wrenFill(WrenCanvas wc, uint32_t color) {
    for (size_t y = 0; y < wc.height; y++) {
        for (size_t x = 0; x < wc.width; x++) {
            WREN_PIXEL(wc, x, y) = color;
        }
    }
}

WRENDEF void wrenRect(WrenCanvas wc, int x, int y, int w, int h, uint32_t color) {
    int x1, y1, x2, y2;
    if (!wrenNormalizeRect(x, y, w, h, wc.width, wc.height, &x1, &x2, &y1, &y2)) return;

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            wrenBlendColors(&WREN_PIXEL(wc, x, y), color);
        }
    }
}

WRENDEF void wrenCircle(WrenCanvas wc, int cx, int cy, int r, uint32_t color) {
    int x1, x2, y1, y2;
    int r1 = r + WREN_SIGN(int, r);
    if (!wrenNormalizeRect(cx - r1, cy - r1, 2*r1, 2*r1, wc.width, wc.height, &x1, &x2, &y1, &y2)) return;

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            int count = 0;
            for (int sox = 0; sox < WREN_AA_RES; sox++) {
                for (int soy = 0; soy < WREN_AA_RES; soy++) {
                    int res1 = (WREN_AA_RES + 1);
                    int dx = (x*res1*2 + 2 + sox*2 - res1*cx*2 - res1);
                    int dy = (y*res1*2 + 2 + soy*2 - res1*cy*2 - res1);
                    if (dx*dx + dy*dy <= res1*res1*r*r*2*2) count += 1;
                }
            }
            uint32_t alpha = ((color&0xFF000000)>>(3*8))*count/WREN_AA_RES/WREN_AA_RES;
            uint32_t updatedColor = (color&0x00FFFFFF)|(alpha<<(3*8));
            wrenBlendColors(&WREN_PIXEL(wc, x, y), updatedColor);
        }
    }
}

WRENDEF void wrenLine(WrenCanvas wc, int x1, int y1, int x2, int y2, uint32_t color) {
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

uint32_t mixColors3(uint32_t c1, uint32_t c2, uint32_t c3, int t1, int t2, int t3, int den) {
    int64_t r1 = WREN_RED(c1);
    int64_t g1 = WREN_GREEN(c1);
    int64_t b1 = WREN_BLUE(c1);
    int64_t a1 = WREN_ALPHA(c1);

    int64_t r2 = WREN_RED(c2);
    int64_t g2 = WREN_GREEN(c2);
    int64_t b2 = WREN_BLUE(c2);
    int64_t a2 = WREN_ALPHA(c2);

    int64_t r3 = WREN_RED(c3);
    int64_t g3 = WREN_GREEN(c3);
    int64_t b3 = WREN_BLUE(c3);
    int64_t a3 = WREN_ALPHA(c3);

    int64_t r4 = (r1*t1 + r2*t2 + r3*t3)/den;
    int64_t g4 = (g1*t1 + g2*t2 + g3*t3)/den;
    int64_t b4 = (b1*t1 + b2*t2 + b3*t3)/den;
    int64_t a4 = (a1*t1 + a2*t2 + a3*t3)/den;

    return WREN_RGBA(r4, g4, b4, a4);
}

void barycentric(int x1, int y1, int x2, int y2, int x3, int y3, int xp, int yp, int *u1, int *u2, int *det) {
    *det = ((x1 - x3)*(y2 - y3) - (x2 - x3)*(y1 - y3));
    *u1  = ((y2 - y3)*(xp - x3) + (x3 - x2)*(yp - y3));
    *u2  = ((y3 - y1)*(xp - x3) + (x1 - x3)*(yp - y3));
}

WRENDEF void wrenTriangle3(WrenCanvas wc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t c1, uint32_t c2, uint32_t c3) {
    if (y1 > y2) {
        WREN_SWAP(int, x1, x2);
        WREN_SWAP(int, y1, y2);
        WREN_SWAP(int, c1, c2);
    }

    if (y2 > y3) {
        WREN_SWAP(int, x2, x3);
        WREN_SWAP(int, y2, y3);
        WREN_SWAP(int, c2, c3);
    }
    
    if (y1 > y2) {
        WREN_SWAP(int, x1, x2);
        WREN_SWAP(int, y1, y2);
        WREN_SWAP(int, c1, c2);
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
                    int u1, u2, det;
                    barycentric(x1, y1, x2, y2, x3, y3, x, y, &u1, &u2, &det);
                    uint32_t color = mixColors3(c1, c2, c3, u1, u2, det - u1 - u2, det);
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
                    int u1, u2, det;
                    barycentric(x1, y1, x2, y2, x3, y3, x, y, &u1, &u2, &det);
                    uint32_t color = mixColors3(c1, c2, c3, u1, u2, det - u1 - u2, det);
                    wrenBlendColors(&WREN_PIXEL(wc, x, y), color);
                }
            }
        }
    }
}

WRENDEF void wrenTriangle(WrenCanvas wc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
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

WRENDEF void wrenText(WrenCanvas wc, const char *text, int tx, int ty, WrenFont font, size_t size, uint32_t color) {
    for (size_t i = 0; *text; i++, text++) {
        int gx = tx + i*font.width*size;
        int gy = ty;
        const char *glyph = &font.glyphs[(*text)*sizeof(char)*font.width*font.height];
        for (int dy = 0; (size_t) dy < font.height; dy++) {
            for (int dx = 0; (size_t) dx < font.width; dx++) {
                int px = gx + dx*size;
                int py = gy + dy*size;
                if (0 <= px && px < (int) wc.width && 0 <= py && py < (int) wc.height) {
                    if (glyph[dy*font.width + dx]) {
                        wrenRect(wc, px, py, size, size, color);
                    }
                }
            }
        }
    }
}

WRENDEF void wrenCopy(WrenCanvas src, WrenCanvas dst) {
    for (size_t y = 0; y < dst.height; y++) {
        for (size_t x = 0; x < dst.width; x++) {
            size_t nx = x*src.width/dst.width;
            size_t ny = y*src.height/dst.height;
            WREN_PIXEL(dst, x, y) = WREN_PIXEL(src, nx, ny);
        }
    }
}

#endif