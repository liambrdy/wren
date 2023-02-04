#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include "wren.c"

#define WIDTH 800
#define HEIGHT 600

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

static uint32_t pixels[HEIGHT*WIDTH];

#define COLS 8
#define ROWS 6
#define CELL_WIDTH (WIDTH/COLS)
#define CELL_HEIGHT (HEIGHT/ROWS)

int main() {
    wrenFill(pixels, WIDTH, HEIGHT, 0xFF202020);

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            uint32_t color;
            if ((x + y) % 2 == 0) {
                color = 0xFF000000;
            } else {
                color = 0xFF00FF00;
            }
            wrenFillRect(pixels, WIDTH, HEIGHT, x*CELL_WIDTH, y*CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT, color);
        }
    }

    const char *filePath = "output.ppm";
    int err = wrenSaveToPPMFile(pixels, WIDTH, HEIGHT, filePath);
    if (err) {
        fprintf(stderr, "ERROR: could not save file %s:%s\n", filePath, strerror(errno));
        return 1;
    }
    return 0;
}