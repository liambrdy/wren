#ifndef WREN_C_
#define WREN_C_

void wrenFill(uint32_t *pixels, size_t width, size_t height, uint32_t color) {
    for (size_t i = 0; i < width*height; i++) {
        pixels[i] = color;
    }
}

#define returnDefer(value) do { result = (value); goto defer; } while (0)

int wrenSaveToPPMFile(uint32_t *pixels, size_t width, size_t height, const char *filePath) {
    int result = 0;
    FILE *f = fopen(filePath, "wb");
    if (f == NULL) returnDefer(errno);

    fprintf(f, "P6\n%zu %zu 255\n", width, height);
    if (ferror(f)) returnDefer(errno);

    for (size_t i = 0; i < width*height; i++) {
        uint32_t pixel = pixels[i];
        uint8_t bytes[3] = {
            (pixel>>(8*0))&0xFF,
            (pixel>>(8*1))&0xFF,
            (pixel>>(8*2))&0xFF
        };
        fwrite(bytes, sizeof(bytes), 1, f);
        if (ferror(f)) returnDefer(errno);
    }

defer:
    if (f) fclose(f);
    return result;
}

#endif