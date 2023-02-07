#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "wren.c"

#define returnDefer(value) do { result = (value); goto defer; } while (0)
#define UNUSED(x) (void)(x)
#define UNIMPLEMENTED(message) \
    do { \
        fprintf(stderr, "%s:%d: UNIMPLEMENTED: %s\n", __FILE__, __LINE__, message); \
        exit(1); \
    } while (0)
#define UNREACHABLE(message) \
    do { \
        fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); \
        exit(1); \
    } while (0)

#define WIDTH 128
#define HEIGHT 128

#define TEST_DIR_PATH "./test"

uint32_t pixels[WIDTH*HEIGHT];

bool recordTestCase(const char *filePath) {
    if (!stbi_write_png(filePath, WIDTH, HEIGHT, 4, pixels, sizeof(uint32_t)*WIDTH)) {
        fprintf(stderr, "ERROR: could not write file: %s: %s\n", filePath, strerror(errno));
        return false;
    }

    printf("Generated %s\n", filePath);
    return true;
}

bool replayTestCase(const char *filePath, const char *failureFilePath) {
    bool result = true;
    uint32_t *expectedPixels = NULL;

    {
        int expectedWidth, expectedHeight;
        expectedPixels = (uint32_t *) stbi_load(filePath, &expectedWidth, &expectedHeight, NULL, 4);
        if (expectedPixels == NULL) {
            fprintf(stderr, "ERROR: could not read file %s: %s\n", filePath, strerror(errno));
            returnDefer(false);
        }

        if (expectedWidth != WIDTH || expectedHeight != HEIGHT) {
            fprintf(stderr, "%s: FAILURE: unexpected image size. Expected %dx%d, but got %dx%d\n",
                    filePath, WIDTH, HEIGHT, expectedWidth, expectedHeight);
            returnDefer(false);
        }

        bool failed = false;
        for (size_t y = 0; y < HEIGHT; y++) {
            for (size_t x = 0; x < WIDTH; x++) {
                uint32_t expectedPixel = expectedPixels[y*WIDTH + x];
                uint32_t actualPixel = pixels[y*WIDTH + x];
                if (expectedPixel != actualPixel) {
                    pixels[y*WIDTH + x] = 0xFF0000FF;
                    failed = true;
                }
            }
        }

        if (failed) {
            fprintf(stderr, "%s: FAILURE: unexpected pixels in generated image\n", filePath);
            if (!stbi_write_png(failureFilePath, WIDTH, HEIGHT, 4, pixels, sizeof(uint32_t)*WIDTH)) {
                fprintf(stderr, "ERROR: could not generate image diff %s: %s\n", failureFilePath, strerror(errno));
            } else {
                printf("See image diff %s for more info\n", failureFilePath);
            }
            returnDefer(false);
        }

        printf("%s OK\n", filePath);
    }

defer:
    if (expectedPixels) stbi_image_free(expectedPixels);
    return result;
}

typedef struct {
    void (*run)(void);
    const char *filePath;
    const char *failureFilePath;
} TestCase;

#define DEFINE_TEST_CASE(name) \
    { \
        .run = name, \
        .filePath = TEST_DIR_PATH "/" #name ".png", \
        .failureFilePath = TEST_DIR_PATH "/" #name "_failure.png" \
    }

void testFillRect() {
    wrenFill(pixels, WIDTH, HEIGHT, 0xFF202020);
    wrenFillRect(pixels, WIDTH, HEIGHT, WIDTH/2 - WIDTH/8, HEIGHT/2 - HEIGHT/8, WIDTH/4, HEIGHT/4, 0xFF2020AA);
    wrenFillRect(pixels, WIDTH, HEIGHT, WIDTH - 1, HEIGHT - 1, -WIDTH/2, -HEIGHT/2, 0xFF20AA20);
    wrenFillRect(pixels, WIDTH, HEIGHT, -WIDTH/4, -HEIGHT/4, WIDTH/2, HEIGHT/2, 0xFFAA2020);
}

void testFillCircle() {
    wrenFill(pixels, WIDTH, HEIGHT, 0xFF202020);
    wrenFillCircle(pixels, WIDTH, HEIGHT, 0, 0, WIDTH/2, 0xFF2020AA);
    wrenFillCircle(pixels, WIDTH, HEIGHT, WIDTH/2, HEIGHT/2, WIDTH/4, 0xFFAA2020);
    wrenFillCircle(pixels, WIDTH, HEIGHT, WIDTH*3/4, HEIGHT*3/4, -WIDTH/4, 0xFF20AA20);
}

TestCase testCases[] = {
    DEFINE_TEST_CASE(testFillRect),
    DEFINE_TEST_CASE(testFillCircle),
};
#define TEST_CASES_COUNT (sizeof(testCases)/sizeof(testCases[0]))

int main(int argc, char **argv) {
    bool record = argc >= 2 && strcmp(argv[1], "record") == 0;

    for (size_t i = 0; i < TEST_CASES_COUNT; i++) {
        testCases[i].run();
        if (record) {
            if (!recordTestCase(testCases[i].filePath)) return 1;
        } else {
            if (!replayTestCase(testCases[i].filePath, testCases[i].failureFilePath)) return 1;
        }
    }

    return 0;
}