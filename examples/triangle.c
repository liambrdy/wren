#include "wren.c"

#define WIDTH 800
#define HEIGHT 600

static uint32_t pixels[WIDTH*HEIGHT];
static float angle = 0;

float sqrtf(float x);
float atan2f(float y, float x);
float sinf(float x);
float cosf(float x);

#define PI 3.14159265359

void rotatePoint(int *x, int *y) {
    int dx = *x - WIDTH/2;
    int dy = *y - HEIGHT/2;
    float mag = sqrtf(dx*dx + dy*dy);
    float dir = atan2f(dy, dx) + angle;
    *x = cosf(dir)*mag + WIDTH/2;
    *y = sinf(dir)*mag + HEIGHT/2;
}

uint32_t *render(float dt) {
    angle += 0.5f*PI*dt;

    wrenFill(pixels, WIDTH, HEIGHT, 0xFF181818);
    {
        int x1 = WIDTH/2, y1 = HEIGHT/8;
        int x2 = WIDTH/8, y2 = HEIGHT/2;
        int x3 = WIDTH*7/8, y3 = HEIGHT*7/8;
        rotatePoint(&x1, &y1);
        rotatePoint(&x2, &y2);
        rotatePoint(&x3, &y3);
        wrenFillTriangle(pixels, WIDTH, HEIGHT, x1, y1, x2, y2, x3, y3, 0xFF2020AA);
    }

    return pixels;
}

#ifdef SDL_PLATFORM
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

#endif