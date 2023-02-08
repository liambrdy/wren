#define WREN_IMPLEMENTATION
#include "wren.c"

float sqrtf(float x);
float atan2f(float y, float x);
float sinf(float x);
float cosf(float x);

#define PI 3.14159265359

#define WIDTH 800
#define HEIGHT 600
#define SCALE_DOWN_FACTOR 10
#define BACKGROUND_COLOR 0xFF181818
#define GRID_COUNT 10
#define GRID_PAD 0.5/GRID_COUNT
#define GRID_SIZE ((GRID_COUNT - 1)*GRID_PAD)
#define CIRCLE_RADIUS 5
#define Z_START 0.25
#define TEXT_PADDING 50

uint32_t circleColors[] = {
    0xFF2020FF,
    0xFF20FF20,
    0xFFFF2020,
    0xFF20FFFF,
    0xFFFF20FF,
    0xFFFFFF20,
};
#define circleColorsCount (sizeof(circleColors)/sizeof(circleColors[0]))

static uint32_t pixels[WIDTH*HEIGHT];
static float angle = 0;

void init() {}

uint32_t *render(float dt) {
    angle += 0.25f*PI*dt;

    WrenCanvas wc = wrenCanvas(pixels, WIDTH, HEIGHT, WIDTH);

    wrenFill(wc, BACKGROUND_COLOR);
    for (int ix = 0; ix < GRID_COUNT; ix++) {
        for (int iy = 0; iy < GRID_COUNT; iy++) {
            for (int iz = 0; iz < GRID_COUNT; iz++) {
                float x = ix*GRID_PAD - GRID_SIZE/2;
                float y = iy*GRID_PAD - GRID_SIZE/2;
                float z = Z_START + iz*GRID_PAD;

                float cx = 0.0f;
                float cz = Z_START + GRID_SIZE/2;

                float dx = x - cx;
                float dz = z - cz;
                
                float a = atan2f(dz, dx);
                float m = sqrtf(dx*dx + dz*dz);

                dx = cosf(a + angle)*m;
                dz = sinf(a + angle)*m;

                x = dx + cx;
                z = dz + cz;

                x /= z;
                y /= z;

                uint32_t r = ix*255/GRID_COUNT;
                uint32_t g = iy*255/GRID_COUNT;
                uint32_t b = iz*255/GRID_COUNT;
                uint32_t color = 0xFF000000 | (r<<(0*8)) | (g<<(1*8)) | (b<<(2*8));
                wrenCircle(wc, (x + 1)/2*WIDTH, (y + 1)/2*HEIGHT, CIRCLE_RADIUS, color);
            }
        }
    }

    size_t size = 8;
    wrenText(wc, "abcd", TEXT_PADDING, HEIGHT - TEXT_PADDING - defaultFont.height*size, defaultFont, size, 0xFFFFFFFF);

    return pixels;
}

#include "vc.c"