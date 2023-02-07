#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WREN_IMPLEMENTATION
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

#define BACKGROUND_COLOR 0xFF202020
#define RED_COLOR 0xFF2020AA
#define GREEN_COLOR 0xFF20AA20
#define BLUE_COLOR 0xFFAA2020
#define ERROR_COLOR 0xFFFF00FF

#define TEST_DIR_PATH "./test"

char hexchar(uint8_t x) {
    if (x < 10) return x + '0';
    if (x < 16) return x - 10 + 'A';
    UNREACHABLE("hexchar");
}

const char *displayHexColor(uint32_t c) {
    static char buffer[1 + 8 + 1];
    buffer[0] = '#';
    buffer[1] = hexchar((c>>(1*4))&0xF);
    buffer[2] = hexchar((c>>(0*4))&0xF);
    buffer[3] = hexchar((c>>(3*4))&0xF);
    buffer[4] = hexchar((c>>(2*4))&0xF);
    buffer[5] = hexchar((c>>(5*4))&0xF);
    buffer[6] = hexchar((c>>(4*4))&0xF);
    buffer[7] = hexchar((c>>(7*4))&0xF);
    buffer[8] = hexchar((c>>(6*4))&0xF);
    buffer[9] = '\0';
    return buffer;
}

static uint32_t actualPixels[WIDTH*HEIGHT];
static uint32_t diffPixels[WIDTH*HEIGHT];

bool recordTestCase(const char *expectedFilePath) {
    if (!stbi_write_png(expectedFilePath, WIDTH, HEIGHT, 4, actualPixels, sizeof(uint32_t)*WIDTH)) {
        fprintf(stderr, "ERROR: could not write file: %s: %s\n", expectedFilePath, strerror(errno));
        return false;
    }

    printf("Generated %s\n", expectedFilePath);
    return true;
}

typedef enum {
    REPLAY_PASSED,
    REPLAY_FAILED,
    REPLAY_ERRORED,
} ReplayResult;

ReplayResult replayTestCase(const char *programPath, const char *expectedFilePath, const char *actualFilePath, const char *diffFilePath) {
    ReplayResult result = REPLAY_PASSED;
    uint32_t *expectedPixels = NULL;

    {
        int expectedWidth, expectedHeight;
        expectedPixels = (uint32_t *) stbi_load(expectedFilePath, &expectedWidth, &expectedHeight, NULL, 4);
        if (expectedPixels == NULL) {
            fprintf(stderr, "%s: ERROR: could not read file: %s\n", expectedFilePath, strerror(errno));
            if (errno == ENOENT) {
                fprintf(stderr, "%s: HINT: Consider running `$ %s record` to create it\n", expectedFilePath, programPath);
            }
            returnDefer(REPLAY_ERRORED);
        }

        if (expectedWidth != WIDTH || expectedHeight != HEIGHT) {
            fprintf(stderr, "%s: TEST FAILURE: unexpected image size. Expected %dx%d, but got %dx%d\n",
                    expectedFilePath, expectedWidth, expectedHeight, WIDTH, HEIGHT);
            returnDefer(REPLAY_FAILED);
        }

        bool failed = false;
        for (size_t y = 0; y < HEIGHT; y++) {
            for (size_t x = 0; x < WIDTH; x++) {
                uint32_t expectedPixel = expectedPixels[y*WIDTH + x];
                uint32_t actualPixel = actualPixels[y*WIDTH + x];
                if (expectedPixel != actualPixel) {
                    diffPixels[y*WIDTH + x] = ERROR_COLOR;
                    failed = true;
                } else {
                    diffPixels[y*WIDTH + x] = expectedPixel;
                }
            }
        }

        if (failed) {
            fprintf(stderr, "%s: TEST FAILURE: unexpected pixels in generated image\n", expectedFilePath);
            if (!stbi_write_png(actualFilePath, WIDTH, HEIGHT, 4, actualPixels, sizeof(uint32_t)*WIDTH)) {
                fprintf(stderr, "ERROR: could not generate image with actual pixels %s: %s\n", actualFilePath, strerror(errno));\
                returnDefer(REPLAY_ERRORED);
            }

            if (!stbi_write_png(diffFilePath, WIDTH, HEIGHT, 4, diffPixels, sizeof(uint32_t)*WIDTH)) {
                fprintf(stderr, "ERROR: could not generate diff image %s: %s\n", diffFilePath, strerror(errno));\
                returnDefer(REPLAY_ERRORED);
            }
            
            fprintf(stderr, "%s: HINT: See actual image %s\n", expectedFilePath, actualFilePath);
            fprintf(stderr, "%s: HINT: See diff image %s\n", expectedFilePath, diffFilePath);
            fprintf(stderr, "%s: HINT: If this behaviour is intentional confirm that by updating the image with `$ %s record`\n", expectedFilePath, programPath);
            returnDefer(REPLAY_FAILED);
        }

        printf("%s OK\n", expectedFilePath);
    }

defer:
    if (expectedPixels) stbi_image_free(expectedPixels);
    return result;
}

typedef struct {
    void (*generateActualPixels)(void);
    const char *expectedFilePath;
    const char *actualFilePath;
    const char *diffFilePath;
} TestCase;

#define DEFINE_TEST_CASE(name) \
    { \
        .generateActualPixels = name, \
        .expectedFilePath = TEST_DIR_PATH "/" #name "_expected.png", \
        .actualFilePath = TEST_DIR_PATH "/" #name "_actual.png", \
        .diffFilePath = TEST_DIR_PATH "/" #name "_diff.png" \
    }

void testFillRect() {
    WrenCanvas wc = wrenCanvas(actualPixels, WIDTH, HEIGHT);
    wrenFill(wc, BACKGROUND_COLOR);
    wrenRect(wc, WIDTH/2 - WIDTH/8, HEIGHT/2 - HEIGHT/8, WIDTH/4, HEIGHT/4, RED_COLOR);
    wrenRect(wc, WIDTH - 1, HEIGHT - 1, -WIDTH/2, -HEIGHT/2, GREEN_COLOR);
    wrenRect(wc, -WIDTH/4, -HEIGHT/4, WIDTH/2, HEIGHT/2, BLUE_COLOR);
}

void testFillCircle() {
    WrenCanvas wc = wrenCanvas(actualPixels, WIDTH, HEIGHT);
    wrenFill(wc, BACKGROUND_COLOR);
    wrenCircle(wc, 0, 0, WIDTH/2, RED_COLOR);
    wrenCircle(wc, WIDTH/2, HEIGHT/2, WIDTH/4, BLUE_COLOR);
    wrenCircle(wc, WIDTH*3/4, HEIGHT*3/4, -WIDTH/4, GREEN_COLOR);
}

void testDrawLine() {
    WrenCanvas wc = wrenCanvas(actualPixels, WIDTH, HEIGHT);
    wrenFill(wc, BACKGROUND_COLOR);
    wrenLine(wc, 0, 0, WIDTH, HEIGHT, RED_COLOR);
    wrenLine(wc, WIDTH, 0, 0, HEIGHT, BLUE_COLOR);
}

void testFillTriangle() {
    WrenCanvas wc = wrenCanvas(actualPixels, WIDTH, HEIGHT);
    wrenFill(wc, BACKGROUND_COLOR);

    {
        int x1 = WIDTH/2, y1 = HEIGHT/8;
        int x2 = WIDTH/8, y2 = HEIGHT/2;
        int x3 = WIDTH*7/8, y3 = HEIGHT*7/8;
        wrenTriangle(wc, x1, y1, x2, y2, x3, y3, RED_COLOR);
    }

    {
        int x1 = WIDTH/2, y1 = HEIGHT*2/8;
        int x2 = WIDTH*2/8, y2 = HEIGHT/2;
        int x3 = WIDTH*6/8, y3 = HEIGHT/2;
        wrenTriangle(wc, x1, y1, x2, y2, x3, y3, GREEN_COLOR);
    }

    {
        int x1 = WIDTH/8, y1 = HEIGHT/8;
        int x2 = WIDTH/8, y2 = HEIGHT*3/8;
        int x3 = WIDTH*3/8, y3 = HEIGHT*3/8;
        wrenTriangle(wc, x1, y1, x2, y2, x3, y3, BLUE_COLOR);
    }
}

void testAlphaBlending() {
    WrenCanvas wc = wrenCanvas(actualPixels, WIDTH, HEIGHT);
    wrenFill(wc, BACKGROUND_COLOR);
    wrenRect(wc, 0, 0, WIDTH*3/4, HEIGHT*3/4, RED_COLOR);
    wrenRect(wc, WIDTH-1, HEIGHT-1, -WIDTH*3/4, -HEIGHT*3/4, 0x5520AA20);
    wrenCircle(wc, WIDTH/2, HEIGHT/2, WIDTH/4, 0xBBAA2020);
    wrenTriangle(wc, 0, HEIGHT, WIDTH, HEIGHT, WIDTH/2, 0, 0xBB20AAAA);
}

TestCase testCases[] = {
    DEFINE_TEST_CASE(testFillRect),
    DEFINE_TEST_CASE(testFillCircle),
    DEFINE_TEST_CASE(testDrawLine),
    DEFINE_TEST_CASE(testFillTriangle),
    DEFINE_TEST_CASE(testAlphaBlending),
};
#define TEST_CASES_COUNT (sizeof(testCases)/sizeof(testCases[0]))

int main(int argc, char **argv) {
    assert(argc >= 1);
    const char *programPath = argv[0];
    bool record = argc >= 2 && strcmp(argv[1], "record") == 0;

    for (size_t i = 0; i < TEST_CASES_COUNT; i++) {
        testCases[i].generateActualPixels();
        if (record) {
            if (!recordTestCase(testCases[i].expectedFilePath)) return 1;
        } else {
            if (replayTestCase(programPath, testCases[i].expectedFilePath, testCases[i].actualFilePath, testCases[i].diffFilePath) == REPLAY_ERRORED) return 1;
        }
    }

    return 0;
}