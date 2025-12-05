#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include "gif_handler.h"


// ------------------------------------------
// Read entire file into memory
// ------------------------------------------
unsigned char* read_file_to_memory(const char* filename, int* out_size)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp)
        return NULL;

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char* buffer = malloc(size);
    if (!buffer)
    {
        fclose(fp);
        return NULL;
    }

    fread(buffer, 1, size, fp);
    fclose(fp);

    *out_size = size;
    return buffer;
}


// ------------------------------------------
// Load GIF using stb_image
// ------------------------------------------
GIFImage load_gif(const char* filename)
{
    GIFImage gif = {0};

    int file_size = 0;
    unsigned char* file_data = read_file_to_memory(filename, &file_size);

    if (!file_data)
    {
        fprintf(stderr, "Failed to read GIF file: %s\n", filename);
        return gif;
    }

    int* delays = NULL;
    int w, h, frames, comp;

    unsigned char* data = stbi_load_gif_from_memory(
        file_data,
        file_size,
        &delays,
        &w,
        &h,
        &frames,
        &comp,
        0   // req_comp = 0 → don't force, use original comp
    );

    free(file_data);

    if (!data)
    {
        fprintf(stderr, "Failed to decode GIF: %s\n", filename);
        return gif;
    }

    printf("GIF INFO: width=%d height=%d frames=%d comp=%d\n",
           w, h, frames, comp);

    gif.frames       = data;
    gif.width        = w;
    gif.height       = h;
    gif.frame_count  = frames;
    gif.delays       = delays;
    gif.frame_stride = w * h * comp;   // bytes per frame

    return gif;
}


// ------------------------------------------
// Get Nth frame (using stride)
// ------------------------------------------
unsigned char* gif_get_frame(GIFImage* gif, int index)
{
    if (index < 0 || index >= gif->frame_count)
        return NULL;

    return gif->frames + (size_t)index * gif->frame_stride;
}


// ------------------------------------------
// Free decoded GIF memory
// ------------------------------------------
void free_gif(GIFImage* gif)
{
    if (gif->frames)
        stbi_image_free(gif->frames);

    if (gif->delays)
        free(gif->delays);
}
