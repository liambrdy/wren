#define WREN_IMPLEMENTATION
#include "wren.c"

float sqrtf(float x);
float atan2f(float y, float x);
float sinf(float x);
float cosf(float x);

#define PI 3.14159265359

#define WIDTH 800
#define HEIGHT 600
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

uint32_t *render(float dt) {
    angle += 0.25f*PI*dt;

    WrenCanvas wc = wrenCanvas(pixels, WIDTH, HEIGHT);

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

#define WASM_PLATFORM 0
#define SDL_PLATFORM 1
#define TERM_PLATFORM 2

#if PLATFORM == SDL_PLATFORM
#include <stdio.h>
#include <SDL2/SDL.h>

#define returnDefer(value) do { result = (value); goto defer; } while (0)

int main() {
    int result = 0;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) returnDefer(1);

        window = SDL_CreateWindow("Wren", 0, 0, WIDTH, HEIGHT, 0);
        if (window == NULL) returnDefer(1);

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == NULL) returnDefer(1);

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
        if (texture == NULL) returnDefer(1);

        Uint32 prev = SDL_GetTicks();
        for (;;) {
            Uint32 curr = SDL_GetTicks();
            float dt = (curr - prev)/1000.0f;
            prev = curr;

            SDL_Event event;
            while (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) returnDefer(0);

            SDL_Rect windowRect = {0, 0, WIDTH, HEIGHT};
            uint32_t *pixelsSrc = render(dt);
            void *pixelsDst;
            int pitch;
            if (SDL_LockTexture(texture, &windowRect, &pixelsDst, &pitch) < 0) returnDefer(1);
            for (size_t y = 0; y < HEIGHT; y++) {
                memcpy(pixelsDst + y*pitch, pixelsSrc + y*WIDTH, WIDTH*sizeof(uint32_t));
            }
            SDL_UnlockTexture(texture);

            if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0) < 0) returnDefer(1);
            if (SDL_RenderClear(renderer) < 0) returnDefer(1);
            if (SDL_RenderCopy(renderer, texture, &windowRect, &windowRect) < 0) returnDefer(1);
            SDL_RenderPresent(renderer);
        }
    }
defer:
    switch (result) {
        case 0:
            printf("OK\n");
            break;
        default:
            fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
    }
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}

#elif PLATFORM == TERM_PLATFORM

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#define SCALE_DOWN_FACTOR 5
static_assert(WIDTH%SCALE_DOWN_FACTOR == 0, "WIDTH must be divisible by the SCALE_DOWN_FACTOR");
#define SCALED_DOWN_WIDTH (WIDTH/SCALE_DOWN_FACTOR)
static_assert(HEIGHT%SCALE_DOWN_FACTOR == 0, "HEIGHT must be divisible by the SCALE_DOWN_FACTOR");
#define SCALED_DOWN_HEIGHT (HEIGHT/SCALE_DOWN_FACTOR)

char charCanvas[SCALED_DOWN_WIDTH*SCALED_DOWN_HEIGHT];

char colorToChar(uint32_t pixel) {
    char table[] = " .:a@#";
    size_t n = sizeof(table) - 1;
    size_t r = (0x000000FF&pixel)>>(8*0);
    size_t g = (0x0000FF00&pixel)>>(8*1);
    size_t b = (0x00FF0000&pixel)>>(8*2);
    size_t bright = r;
    if (bright < g) bright = g;
    if (bright < b) bright = b;
    return table[bright*n/256];
}

uint32_t compressPixelsChunk(WrenCanvas wc) {
    size_t r = 0;
    size_t g = 0;
    size_t b = 0;
    size_t a = 0;

    for (size_t y = 0; y < wc.height; ++y) {
        for (size_t x = 0; x < wc.width; ++x) {
            r += WREN_RED(WREN_PIXEL(wc, x, y));
            g += WREN_GREEN(WREN_PIXEL(wc, x, y));
            b += WREN_BLUE(WREN_PIXEL(wc, x, y));
            a += WREN_ALPHA(WREN_PIXEL(wc, x, y));
        }
    }

    r /= wc.width*wc.height;
    g /= wc.width*wc.height;
    b /= wc.width*wc.height;
    a /= wc.width*wc.height;

    return WREN_RGBA(r, g, b, a);
}

void compressPixels(uint32_t *pixels) {
    WrenCanvas wc = wrenCanvas(pixels, WIDTH, HEIGHT);
    for (size_t y = 0; y < SCALED_DOWN_HEIGHT; y++) {
        for (size_t x = 0; x < SCALED_DOWN_WIDTH; x++) {
            WrenCanvas swc = wrenSubcanvas(wc, x*SCALE_DOWN_FACTOR, y*SCALE_DOWN_FACTOR, SCALE_DOWN_FACTOR, SCALE_DOWN_FACTOR);
            charCanvas[y*SCALED_DOWN_WIDTH + x] = colorToChar(compressPixelsChunk(swc));
        }
    }
}

int main(void)
{
    for (;;) {
        compressPixels(render(1.f/60.f));
        for (size_t y = 0; y < SCALED_DOWN_HEIGHT; ++y) {
            for (size_t x = 0; x < SCALED_DOWN_WIDTH; ++x) {
                putc(charCanvas[y*SCALED_DOWN_WIDTH + x], stdout);
                putc(charCanvas[y*SCALED_DOWN_WIDTH + x], stdout);
            }
            putc('\n', stdout);
        }

        usleep(1000*1000/60);
        printf("\033[%dA", SCALED_DOWN_HEIGHT);
        printf("\033[%dD", SCALED_DOWN_WIDTH);
    }
    return 0;
}

#elif PLATFORM == WASM_PLATFORM
#else
#error "Unknown platform
#endif