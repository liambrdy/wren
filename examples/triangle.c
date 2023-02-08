#define WREN_IMPLEMENTATION
#include "wren.c"

#define WIDTH 800
#define HEIGHT 600
#define SCALE_DOWN_FACTOR 20
#define BACKGROUND_COLOR 0xFF181818
#define CIRCLE_RADIUS 100
#define CIRCLE_COLOR 0x99AA2020

static uint32_t pixels[WIDTH*HEIGHT];
static float triangleAngle = 0;
static float circleX = WIDTH/2;
static float circleY = HEIGHT/2;
static float circleDx = 100;
static float circleDy = 100;

float sqrtf(float x);
float atan2f(float y, float x);
float sinf(float x);
float cosf(float x);

#define PI 3.14159265359

static inline void rotatePoint(float *x, float *y) {
    float dx = *x - WIDTH/2;
    float dy = *y - HEIGHT/2;
    float mag = sqrtf(dx*dx + dy*dy);
    float dir = atan2f(dy, dx) + triangleAngle;
    *x = cosf(dir)*mag + WIDTH/2;
    *y = sinf(dir)*mag + HEIGHT/2;
}

void init() {}

uint32_t *render(float dt) {
    WrenCanvas wc = wrenCanvas(pixels, WIDTH, HEIGHT, WIDTH);

    wrenFill(wc, 0xFF181818);
    {
        triangleAngle += 0.5f*PI*dt;

        float x1 = WIDTH/2, y1 = HEIGHT/8;
        float x2 = WIDTH/8, y2 = HEIGHT/2;
        float x3 = WIDTH*7/8, y3 = HEIGHT*7/8;
        rotatePoint(&x1, &y1);
        rotatePoint(&x2, &y2);
        rotatePoint(&x3, &y3);

        wrenTriangle3(wc, x1, y1, x2, y2, x3, y3, 0xFF2020FF, 0xFF20FF20, 0xFFFF2020);
    }

    {
        float x = circleX + circleDx*dt;
        if (x - CIRCLE_RADIUS < 0 || x + CIRCLE_RADIUS >= WIDTH) {
            circleDx *= -1;
        } else {
            circleX = x;
        }

        float y = circleY + circleDy*dt;
        if (y - CIRCLE_RADIUS < 0 || y + CIRCLE_RADIUS >= HEIGHT) {
            circleDy *= -1;
        } else {
            circleY = y;
        }

        wrenCircle(wc, circleX, circleY, CIRCLE_RADIUS, CIRCLE_COLOR);
    }

    return pixels;
}

#include "vc.c"