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
        init();

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

static_assert(WIDTH%SCALE_DOWN_FACTOR == 0, "WIDTH must be divisible by the SCALE_DOWN_FACTOR");
#define SCALED_DOWN_WIDTH (WIDTH/SCALE_DOWN_FACTOR)
static_assert(HEIGHT%SCALE_DOWN_FACTOR == 0, "HEIGHT must be divisible by the SCALE_DOWN_FACTOR");
#define SCALED_DOWN_HEIGHT (HEIGHT/SCALE_DOWN_FACTOR)

char charCanvas[SCALED_DOWN_WIDTH*SCALED_DOWN_HEIGHT];

char colorToChar(uint32_t pixel) {
    size_t r = WREN_RED(pixel);
    size_t g = WREN_GREEN(pixel);
    size_t b = WREN_BLUE(pixel);
    size_t a = WREN_ALPHA(pixel);

    size_t bright = r;
    if (bright < g) bright = g;
    if (bright < b) bright = b;
    bright = bright*a/255;

    char table[] = " .:a@#";
    size_t n = sizeof(table) - 1;
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
    WrenCanvas wc = wrenCanvas(pixels, WIDTH, HEIGHT, WIDTH);
    for (size_t y = 0; y < SCALED_DOWN_HEIGHT; y++) {
        for (size_t x = 0; x < SCALED_DOWN_WIDTH; x++) {
            WrenCanvas swc = wrenSubcanvas(wc, x*SCALE_DOWN_FACTOR, y*SCALE_DOWN_FACTOR, SCALE_DOWN_FACTOR, SCALE_DOWN_FACTOR);
            charCanvas[y*SCALED_DOWN_WIDTH + x] = colorToChar(compressPixelsChunk(swc));
        }
    }
}

int main(void)
{
    init();

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