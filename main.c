#include "png_handler.h"
#include <string.h>
#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <turbojpeg.h>
#include "gif_handler.h"
#include <unistd.h>   // for usleep()


// Import specific OS-specific headers.
#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#endif

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

int verbose(const char *restrict format, ...)
{
    if (!verbose_mode)
        return 0;

    va_list args;
    va_start(args, format);
    int ret = vfprintf(stderr, format, args);
    va_end(args);
    return ret;
}

void get_terminal_size(int *width, int *height)
{
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    *width = (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    *height = (int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
#elif defined(__linux__)
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    *width = (int)(w.ws_col);
    *height = (int)(w.ws_row);
#endif
}

// -------------------------------------------------------------
// JPEG Processing Function
// -------------------------------------------------------------
static int process_jpeg(FILE *file, const int target_width, const int target_height)
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

    int jpeg_width = tj3Get(tj, TJPARAM_JPEGWIDTH);
    int jpeg_height = tj3Get(tj, TJPARAM_JPEGHEIGHT);
    verbose("Image dimensions (px): %dx%d\n", jpeg_width, jpeg_height);

    unsigned char *rgb_buffer = malloc(3 * jpeg_width * jpeg_height);
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

    // Calculate target scale
    float x_scale = (float)jpeg_width / target_width;
    float y_scale = (float)jpeg_height / target_height;

    // Render JPEG
    // Each terminal cell displays two vertically-stacked pixels:
    // - Top pixel via background color (48;2)
    // - Bottom pixel via foreground color (38;2)

    int terminal_rows = target_height / 2;
    for (int y = 0; y < terminal_rows; y++)
    {
        for (int x = 0; x < target_width; x++)
        {
            // Sample top pixel
            int src_x = (int)(x * x_scale);
            int src_y_top = (int)((y * 2) * y_scale);
            src_x = (src_x < jpeg_width) ? src_x : jpeg_width - 1;
            src_y_top = (src_y_top < jpeg_height) ? src_y_top : jpeg_height - 1;

            int pixel_index_top = 3 * (jpeg_width * src_y_top + src_x);
            unsigned char r_top = rgb_buffer[pixel_index_top];
            unsigned char g_top = rgb_buffer[pixel_index_top + 1];
            unsigned char b_top = rgb_buffer[pixel_index_top + 2];

            // Sample bottom pixel
            int src_y_bottom = (int)((y * 2 + 1) * y_scale);
            src_y_bottom = (src_y_bottom < jpeg_height) ? src_y_bottom : jpeg_height - 1;

            int pixel_index_bottom = 3 * (jpeg_width * src_y_bottom + src_x);
            unsigned char r_bottom = rgb_buffer[pixel_index_bottom];
            unsigned char g_bottom = rgb_buffer[pixel_index_bottom + 1];
            unsigned char b_bottom = rgb_buffer[pixel_index_bottom + 2];

            // Print half-block with two colors
            printf("\x1b[48;2;%d;%d;%d;38;2;%d;%d;%dm▄",
                   r_top, g_top, b_top,
                   r_bottom, g_bottom, b_bottom);
        }
        printf("\033[0m\n");
    }

    // Handle odd target_height: render final row if there's an odd pixel left
    if (target_height % 2 != 0)
    {
        int y = terminal_rows;
        for (int x = 0; x < target_width; x++)
        {
            // Sample the last row pixel
            int src_x = (int)(x * x_scale);
            int src_y = (int)((y * 2) * y_scale);
            src_x = (src_x < jpeg_width) ? src_x : jpeg_width - 1;
            src_y = (src_y < jpeg_height) ? src_y : jpeg_height - 1;

            int pixel_index = 3 * (jpeg_width * src_y + src_x);
            unsigned char r = rgb_buffer[pixel_index];
            unsigned char g = rgb_buffer[pixel_index + 1];
            unsigned char b = rgb_buffer[pixel_index + 2];

            // Print half-block with the pixel as background and black as foreground
            printf("\x1b[48;2;%d;%d;%d;38;2;0;0;0m▄", r, g, b);
        }
        printf("\033[0m\n");
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
void render_png_downscaled(PNGImage img, int target_width, int target_height)
{
    int src_width = img.width;
    int src_height = img.height;
    unsigned char *pixels = img.pixels; // RGBA

    float x_scale = (float)src_width / target_width;
    float y_scale = (float)src_height / target_height;

    int terminal_rows = target_height / 2;

    for (int y = 0; y < terminal_rows; y++)
    {
        for (int x = 0; x < target_width; x++)
        {
            int sx = (int)(x * x_scale);

            /* ---- Sample TOP pixel ---- */
            int sy_top = (int)((y * 2) * y_scale);
            if (sy_top >= src_height) sy_top = src_height - 1;
            if (sx >= src_width) sx = src_width - 1;

            int top_idx = 4 * (sy_top * src_width + sx);
            unsigned char r_top = pixels[top_idx + 0];
            unsigned char g_top = pixels[top_idx + 1];
            unsigned char b_top = pixels[top_idx + 2];

            /* ---- Sample BOTTOM pixel ---- */
            int sy_bot = (int)((y * 2 + 1) * y_scale);
            if (sy_bot >= src_height) sy_bot = src_height - 1;

            int bot_idx = 4 * (sy_bot * src_width + sx);
            unsigned char r_bot = pixels[bot_idx + 0];
            unsigned char g_bot = pixels[bot_idx + 1];
            unsigned char b_bot = pixels[bot_idx + 2];

            printf("\x1b[48;2;%d;%d;%d;38;2;%d;%d;%d" "m▄",
                   r_top, g_top, b_top,
                   r_bot, g_bot, b_bot);
        }
        printf("\033[0m\n");
    }

    /* Render last odd row if height is odd */
    if (target_height % 2 != 0)
    {
        int y = terminal_rows;

        for (int x = 0; x < target_width; x++)
        {
            int sx = (int)(x * x_scale);

            int sy = (int)((y * 2) * y_scale);
            if (sy >= src_height) sy = src_height - 1;
            if (sx >= src_width) sx = src_width - 1;

            int idx = 4 * (sy * src_width + sx);
            unsigned char r = pixels[idx + 0];
            unsigned char g = pixels[idx + 1];
            unsigned char b = pixels[idx + 2];

            printf("\x1b[48;2;%d;%d;%d;38;2;0;0;0m▄", r, g, b);
        }
        printf("\033[0m\n");
    }
}
void render_frame_downscaled(unsigned char* pixels,
                             int src_width,
                             int src_height,
                             int target_width,
                             int target_height)
{
    float x_scale = (float)src_width / target_width;
    float y_scale = (float)src_height / target_height;

    int terminal_rows = target_height / 2;

    for (int y = 0; y < terminal_rows; y++)
    {
        for (int x = 0; x < target_width; x++)
        {
            int sx = (int)(x * x_scale);

            // TOP pixel
            int sy_top = (int)((y * 2) * y_scale);
            if (sx >= src_width) sx = src_width - 1;
            if (sy_top >= src_height) sy_top = src_height - 1;

            int idx_top = 4 * (src_width * sy_top + sx);
            unsigned char r_top = pixels[idx_top + 0];
            unsigned char g_top = pixels[idx_top + 1];
            unsigned char b_top = pixels[idx_top + 2];

            // BOTTOM pixel
            int sy_bottom = (int)((y * 2 + 1) * y_scale);
            if (sy_bottom >= src_height) sy_bottom = src_height - 1;

            int idx_bottom = 4 * (src_width * sy_bottom + sx);
            unsigned char r_bottom = pixels[idx_bottom + 0];
            unsigned char g_bottom = pixels[idx_bottom + 1];
            unsigned char b_bottom = pixels[idx_bottom + 2];

            printf("\x1b[48;2;%d;%d;%d;38;2;%d;%d;%dm▄",
                   r_top, g_top, b_top,
                   r_bottom, g_bottom, b_bottom);
        }
        printf("\033[0m\n");
    }
}
// -------------------------------------------------------------
// MAIN FUNCTION
// -------------------------------------------------------------
int main(int argc, char const *argv[])
{
    FILE *file = stdin;

    // Terminal width and height
    int width;
    int height;
    get_terminal_size(&width, &height);
    verbose("Terminal dimensions: %d x %d\n", width, height);

    // Command line parsing
    bool skip_next = false;
    for (int i = 1; i < argc; i++)
    {
        if (skip_next)
        {
            skip_next = false;
            continue;
        }

        const char *arg = argv[i];
        printf("ARG[%d] = %s\n", i, arg);

        

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

        if (strcmp(arg, "-w") == 0 || strcmp(arg, "--width") == 0)
        {
            width = strtol(argv[i + 1], NULL, 10);
            verbose("Setting width to %d\n", width);
            skip_next = true;
            continue;
        }

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--height") == 0)
        {
            height = strtol(argv[i + 1], NULL, 10);
            verbose("Setting height to %d\n", height);
            skip_next = true;
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
    // DEBUG CHECK
    printf("DEBUG: filename = '%s'\n", filename);
    printf("DEBUG: ends_with(.gif) = %d\n", ends_with(filename, ".gif"));
    printf("DEBUG: ends_with(.png) = %d\n", ends_with(filename, ".png"));
    printf("DEBUG: ends_with(.jpg) = %d\n", ends_with(filename, ".jpg"));
    printf("DEBUG: ends_with(.jpeg) = %d\n", ends_with(filename, ".jpeg"));

    if (ends_with(filename, ".png"))
    {
        PNGImage img = load_png(filename);
        if (!img.pixels)
        {
            fprintf(stderr, "Failed to load PNG image.\n");
            return EXIT_FAILURE;
        }

        render_png_downscaled(img, width, height);

        free(img.pixels);
    }
    else if (ends_with(filename, ".jpg") || ends_with(filename, ".jpeg"))
    {
        process_jpeg(file, width, height);
    }
   else if (ends_with(filename, ".gif"))
{
    GIFImage gif = load_gif(filename);
    if (!gif.frames)
    {
        fprintf(stderr, "Failed to load GIF image.\n");
        return EXIT_FAILURE;
    }

    verbose("Loaded GIF %s: %dx%d, %d frames\n",
            filename, gif.width, gif.height, gif.frame_count);

    printf("Press ENTER to show GIF...\n");
    getchar();

    
    printf("\033[?25l");

    for (int f = 0; f < gif.frame_count; f++)
    {
        // Hide cursor for clean animation
        printf("\033[?25l");

        // 🔥 Proper full clear (including scrollback)
        printf("\033[2J\033[H\033[3J");

        unsigned char* frame = gif_get_frame(&gif, f);
        if (!frame) continue;

        render_frame_downscaled(
            frame,
            gif.width,
            gif.height,
            width,
            height
        );

        fflush(stdout);

        // Show debug ONLY in verbose mode
        if (verbose_mode)
            printf("Frame %d raw_delay=%d cs (centiseconds)\n", 
                f, gif.delays ? gif.delays[f] : -1);

        // Delay handling
        int raw_delay = gif.delays ? gif.delays[f] : 5;   // centiseconds
        int delay_ms = raw_delay * 10;                    // convert to ms

        // Smooth FPS control
        if (delay_ms > 80) delay_ms = 80;   // max ~12 FPS
        if (delay_ms < 30) delay_ms = 30;   // min ~33 FPS

        usleep(delay_ms * 1000);
    }

    // Restore cursor after done
    printf("\033[?25h");

    free_gif(&gif);
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
