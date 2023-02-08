#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WREN_IMPLEMENTATION
#include "wren.c"

#define WIDTH 800
#define HEIGHT 600

#define COLS (8 * 2)
#define ROWS (6 * 2)
#define CELL_WIDTH (WIDTH/COLS)
#define CELL_HEIGHT (HEIGHT/ROWS)

#define BACKGROUND_COLOR 0xFF202020
#define FOREGROUND_COLOR 0xFF2020FF

#define IMGS_DIR_PATH "./imgs"

static uint32_t pixels[HEIGHT*WIDTH];

bool checkerExample() {
    WrenCanvas wc = wrenCanvas(pixels, WIDTH, HEIGHT, WIDTH);
    wrenFill(wc, 0xFF202020);

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            uint32_t color = BACKGROUND_COLOR;
            if ((x + y) % 2 == 0) {
                color = 0xFF0000FF;
            }
            wrenRect(wc, x*CELL_WIDTH, y*CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT, color);
        }
    }

    const char *filePath = IMGS_DIR_PATH"/checker.png";
    printf("Generated %s\n", filePath);
    if (!stbi_write_png(filePath, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t))) {
        fprintf(stderr, "ERROR: could not save file %s:%s\n", filePath, strerror(errno));
        return false;
    }

    return true;
}

float lerpf(float a, float b, float t) {
    return a + (b - a)*t;
}

bool circleExample() {
    WrenCanvas wc = wrenCanvas(pixels, WIDTH, HEIGHT, WIDTH);
    wrenFill(wc, BACKGROUND_COLOR);

    for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
            float u = (float)x / COLS;
            float v = (float)y / ROWS;
            float t = (u + v) / 2;

            size_t radius = CELL_WIDTH;
            if (CELL_HEIGHT < radius) radius = CELL_HEIGHT;
            wrenCircle(wc, x*CELL_WIDTH + CELL_WIDTH/2, y*CELL_HEIGHT + CELL_HEIGHT/2, lerpf(radius / 4, radius / 2, t), FOREGROUND_COLOR);
        }
    }

    const char *filePath = IMGS_DIR_PATH"/circle.png";
    printf("Generated %s\n", filePath);
    if (!stbi_write_png(filePath, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t))) {
        fprintf(stderr, "ERROR: could not save file %s:%s\n", filePath, strerror(errno));
        return false;
    }

    return true;
}

bool lineExample() {
    WrenCanvas wc = wrenCanvas(pixels, WIDTH, HEIGHT, WIDTH);
    wrenFill(wc, BACKGROUND_COLOR);

    wrenLine(wc, 0, 0, WIDTH, HEIGHT, FOREGROUND_COLOR);
    wrenLine(wc, WIDTH, 0, 0, HEIGHT, FOREGROUND_COLOR);

    wrenLine(wc, 0, 0, WIDTH/4, HEIGHT, 0xFF20FF20);
    wrenLine(wc, WIDTH/4, 0, 0, HEIGHT, 0xFF20FF20);

    wrenLine(wc, WIDTH, 0, WIDTH/4*3, HEIGHT, 0xFF20FF20);
    wrenLine(wc, WIDTH/4*3, 0, WIDTH, HEIGHT, 0xFF20FF20);

    wrenLine(wc, 0, HEIGHT/2, WIDTH, HEIGHT/2, 0xFFFF2020);
    wrenLine(wc, WIDTH/2, 0, WIDTH/2, HEIGHT, 0xFFFF2020);

    const char *filePath = IMGS_DIR_PATH"/lines.png";
    printf("Generated %s\n", filePath);
    if (!stbi_write_png(filePath, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t))) {
        fprintf(stderr, "ERROR: could not save file %s:%s\n", filePath, strerror(errno));
        return false;
    }

    return true;
}

bool triangleExample() {
    WrenCanvas wc = wrenCanvas(pixels, WIDTH, HEIGHT, WIDTH);
    wrenFill(wc, BACKGROUND_COLOR);

    wrenTriangle(wc, 0, 0, WIDTH, 0, WIDTH/2, HEIGHT, FOREGROUND_COLOR);

    const char *filePath = IMGS_DIR_PATH"/triangles.png";
    printf("Generated %s\n", filePath);
    if (!stbi_write_png(filePath, WIDTH, HEIGHT, 4, pixels, WIDTH*sizeof(uint32_t))) {
        fprintf(stderr, "ERROR: could not save file %s:%s\n", filePath, strerror(errno));
        return false;
    }

    return true;
}

int main() {
    if (!checkerExample()) return -1;
    if (!circleExample()) return -1;
    if (!lineExample()) return -1;
    if (!triangleExample()) return -1;

    return 0;
}