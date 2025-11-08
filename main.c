#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <turbojpeg.h>

static void print_help()
{
    // TODO https://bettercli.org/design/cli-help-page/
    printf("Usage:\n"
           "  vishellize [file] [...]\n"
           "  vishellize [-v | --verbose] [file] [...] -- Display debug logs.\n"
           "  vishellize [-h | --help] [...] -- Shows this help page.\n");
}

static size_t get_file_size(FILE *file)
{
    long current_position = ftell(file);

    fseek(file, 0, SEEK_END);
    unsigned long length = ftell(file);
    fseek(file, current_position, SEEK_SET);

    return length;
}

// https://stackoverflow.com/questions/36095915/implementing-verbose-in-c
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

static void process_jpeg(FILE *file)
{
    size_t jpeg_size = get_file_size(file);
    verbose("File size: %ld\n", jpeg_size);

    unsigned char *jpeg_buffer = malloc(jpeg_size);
    if (jpeg_buffer == NULL)
    {
        fprintf(stderr, "Couldn't allocate memory for file.\n");
        return;
    }

    const size_t bytes_read = fread(jpeg_buffer, 1, jpeg_size, file);
    verbose("Read %d bytes into buffer.\n", bytes_read);
    if (bytes_read != jpeg_size)
    {
        fprintf(stderr, "Couldn't load file contents into memory.\n");
        goto cleanup;
    }

    tjhandle tj = tj3Init(TJINIT_DECOMPRESS);
    if (tj == NULL)
    {
        fprintf(stderr, "Couldn't create TurboJPEG instance: %s.\n", tj3GetErrorStr(tj));
        goto cleanup2;
    }

    if (tj3DecompressHeader(tj, jpeg_buffer, jpeg_size) < 0)
    {
        fprintf(stderr, "Couldn't decompress JPEG header: %s.\n", tj3GetErrorStr(tj));
        goto cleanup2;
    }

    int width = tj3Get(tj, TJPARAM_JPEGWIDTH);
    int height = tj3Get(tj, TJPARAM_JPEGHEIGHT);
    verbose("Image dimensions (px): %dx%d\n", width, height);

cleanup2:
    tj3Destroy(tj);
cleanup:
    free(jpeg_buffer);
}

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

        // TODO: Any future flags should `continue` or `return`.

        if (strlen(arg) > 1 && strncmp(arg, "-", 1) == 0)
        {
            fprintf(stderr, "Invalid flag '%s'.\n", arg);
            return EXIT_FAILURE;
        }

        if (strlen(arg) > 2 && strncmp(arg, "--", 2) == 0)
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

    // TODO: Determine file type by checking file signatures.
    //       https://en.wikipedia.org/wiki/List_of_file_signatures
    //       For example, PNG files always start with 89 50 4E 47 0D 0A 1A 0A.

    process_jpeg(file);

    // Clean up

    if (file != NULL && file != stdin)
        fclose(file);

    return EXIT_SUCCESS;
}
