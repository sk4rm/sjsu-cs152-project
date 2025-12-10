#ifndef GIF_HANDLER_H
#define GIF_HANDLER_H

typedef struct
{
    unsigned char *frames; // All frames in one block (RGBA)
    int width;
    int height;
    int frame_count;
    int *delays; // milliseconds per frame
    int frame_stride;
} GIFImage;

GIFImage load_gif(const char *filename);
unsigned char *gif_get_frame(GIFImage *gif, int index);
void free_gif(GIFImage *gif);

#endif