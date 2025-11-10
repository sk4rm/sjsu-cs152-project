#include "png_handler.h"
#include <string.h>
#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <turbojpeg.h>

// -------------------------------------------------------------
// Helper: Print help menu
// -------------------------------------------------------------
static void print_help()
{
    printf("Usage:\n"
           "  vishellize [file] [...]\n"
           "  vishellize [-v | --verbose] [file] [...] -- Display debug logs.\n"
           "  vishellize [-h | --help] [...] -- Shows this help page.\n");
}

// -------------------------------------------------------------
// Helper: Get file size
// -------------------------------------------------------------
static size_t get_file_size(FILE *file)
{
    long current_position = ftell(file);
    fseek(file, 0, SEEK_END);
    unsigned long length = ftell(file);
    fseek(file, current_position, SEEK_SET);
    return length;
}

// -------------------------------------------------------------
// Verbose logging
// -------------------------------------------------------------
bool verbose_mode = false;

static int verbose(const char *restrict format, ...)
{
    if (!verbose_mode)
        return 0;

    va_list args;
    va_start(args, format);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

// -------------------------------------------------------------
// JPEG Processing Function
// -------------------------------------------------------------
static int process_jpeg(FILE *file)
{
    size_t jpeg_size = get_file_size(file);
    verbose("File size: %ld\n", jpeg_size);

    unsigned char *jpeg_buffer = malloc(jpeg_size);
    if (jpeg_buffer == NULL)
    {
        fprintf(stderr, "Couldn't allocate memory for file.\n");
        return 1;
    }

    const size_t bytes_read = fread(jpeg_buffer, 1, jpeg_size, file);
    verbose("Read %d bytes into buffer.\n", bytes_read);

    if (bytes_read != jpeg_size)
    {
        fprintf(stderr, "Couldn't load file contents into memory.\n");
        free(jpeg_buffer);
        return 1;
    }

    tjhandle tj = tj3Init(TJINIT_DECOMPRESS);
    if (tj == NULL)
    {
        fprintf(stderr, "Couldn't create TurboJPEG instance: %s.\n", tj3GetErrorStr(tj));
        free(jpeg_buffer);
        return 1;
    }

    if (tj3DecompressHeader(tj, jpeg_buffer, jpeg_size) < 0)
    {
        fprintf(stderr, "Couldn't decompress JPEG header: %s.\n", tj3GetErrorStr(tj));
        tj3Destroy(tj);
        free(jpeg_buffer);
        return 1;
    }

    int width = tj3Get(tj, TJPARAM_JPEGWIDTH);
    int height = tj3Get(tj, TJPARAM_JPEGHEIGHT);
    verbose("Image dimensions (px): %dx%d\n", width, height);

    unsigned char *rgb_buffer = malloc(3 * width * height);
    if (rgb_buffer == NULL)
    {
        fprintf(stderr, "Couldn't allocate memory for RGB buffer.\n");
        tj3Destroy(tj);
        free(jpeg_buffer);
        return 1;
    }

    if (tj3Decompress8(tj, jpeg_buffer, jpeg_size, rgb_buffer, 0, TJPF_RGB))
    {
        fprintf(stderr, "Couldn't decompress image into RGB buffer: %s.\n", tj3GetErrorStr(tj));
        free(rgb_buffer);
        tj3Destroy(tj);
        free(jpeg_buffer);
        return 1;
    }

    // Render JPEG
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int pixel_index = 3 * (width * y + x);
            unsigned char r = rgb_buffer[pixel_index];
            unsigned char g = rgb_buffer[pixel_index + 1];
            unsigned char b = rgb_buffer[pixel_index + 2];
            printf("\x1b[38;2;%d;%d;%dm█", r, g, b);
        }
        printf("\n");
    }

    printf("\033[0m"); // Reset ANSI colors

    free(rgb_buffer);
    tj3Destroy(tj);
    free(jpeg_buffer);

    return 0;
}

// -------------------------------------------------------------
// Helper: Check if string ends with suffix
// -------------------------------------------------------------
int ends_with(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

// -------------------------------------------------------------
// MAIN FUNCTION
// -------------------------------------------------------------
int main(int argc, char const *argv[])
{
    FILE *file = stdin;

    // Command line parsing
    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            print_help();
            return EXIT_SUCCESS;
        }

        if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0)
        {
            verbose_mode = true;
            continue;
        }

        if (strlen(arg) > 1 && strncmp(arg, "-", 1) == 0)
        {
            fprintf(stderr, "Invalid flag '%s'.\n", arg);
            return EXIT_FAILURE;
        }

        file = fopen(arg, "rb");
        if (file == NULL)
        {
            fprintf(stderr, "Couldn't open file '%s'.\n", arg);
            return EXIT_FAILURE;
        }
        break;
    }

    setlocale(LC_CTYPE, "en_us.UTF8"); // Unicode handling

    // Detect file type by extension
    const char *filename = argv[argc - 1];

    if (ends_with(filename, ".png"))
    {
        PNGImage img = load_png(filename);
        if (!img.pixels)
        {
            fprintf(stderr, "Failed to load PNG image.\n");
            return EXIT_FAILURE;
        }

        // Render PNG
        for (int y = 0; y < img.height; y++)
        {
            unsigned char *row = img.pixels + y * img.width * 4; // 4 bytes per pixel (RGBA)
            for (int x = 0; x < img.width; x++)
            {
                unsigned char *px = &row[x * 4];
                unsigned char r = px[0];
                unsigned char g = px[1];
                unsigned char b = px[2];
                printf("\x1b[38;2;%d;%d;%dm█", r, g, b);
            }
            printf("\x1b[0m\n");
        }

        free(img.pixels);
    }
    else if (ends_with(filename, ".jpg") || ends_with(filename, ".jpeg"))
    {
        process_jpeg(file);
    }
    else
    {
        fprintf(stderr, "Unsupported file format.\n");
        return EXIT_FAILURE;
    }

    if (file != NULL && file != stdin)
        fclose(file);

    return EXIT_SUCCESS;
}
