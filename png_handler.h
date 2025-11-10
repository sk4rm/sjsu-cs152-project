#ifndef PNG_HANDLER_H
#define PNG_HANDLER_H

typedef struct {
    unsigned char *pixels;
    int width;
    int height;
} PNGImage;

PNGImage load_png(const char *filename);

#endif
