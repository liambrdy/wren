#define WREN_IMPLEMENTATION
#include "wren.c"

#include "assets/bird.c"

#define WIDTH 800
#define HEIGHT 600
#define SCALE_DOWN_FACTOR 10

float sinf(float);

uint32_t dst[WIDTH*HEIGHT];
float globalTime = 0;

void init() {}

#define SRC_SCALE 3

uint32_t *render(float dt) {
    globalTime += dt;

    float t = sinf(10*globalTime);

    wrenFill(wrenCanvas(dst, WIDTH, HEIGHT, WIDTH), 0xFF181818);
    WrenCanvas dstCanvas = wrenCanvas(dst, WIDTH, HEIGHT, WIDTH);

    int factor = 100;
    int w = png_width*SRC_SCALE - t*factor;
    int h = png_height*SRC_SCALE + t*factor;

    wrenCopy(wrenCanvas(png, png_width, png_height, png_width), wrenSubcanvas(dstCanvas, WIDTH/2 - w/2, HEIGHT - h, h, h));

    return dst;
}

#include "vc.c"