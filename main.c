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

static unsigned long get_file_size(FILE *file)
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
    unsigned long jpeg_size = get_file_size(file);
    verbose("File size: %ld\n", jpeg_size);

    char *jpeg_buffer = malloc(jpeg_size);
    if (jpeg_buffer == NULL)
    {
        fprintf(stderr, "Couldn't allocate memory for file.\n");
        return;
    }

    if (fread(jpeg_buffer, 1, jpeg_size, file) != jpeg_size)
    {
        fprintf(stderr, "Couldn't load file contents into memory.\n");
        return;
    }

    tjhandle decompressor = tjInitDecompress();
    tjDestroy(decompressor);

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

        file = fopen(arg, "r");
        if (file == NULL)
        {
            fprintf(stderr, "Couldn't open file '%s'\n", argv[1]);
            return EXIT_FAILURE;
        }
        break;
    }

    process_jpeg(file);

    // Clean up

    if (file != NULL && file != stdin)
        fclose(file);

    return EXIT_SUCCESS;
}
