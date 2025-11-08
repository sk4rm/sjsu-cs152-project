#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_help()
{
    // TODO https://bettercli.org/design/cli-help-page/
    printf("Usage:\n"
           "  vishellize [file] [...]\n"
           "  vishellize [-h | --help] [...] -- Shows this help page.");
}

int main(int argc, char const *argv[])
{
    FILE *file = stdin;
    bool should_file_close = false;

    // Command line parsing

    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            print_help();
            return EXIT_SUCCESS;
        }

        // TODO: Any future flags should `continue` or `return`.

        file = fopen(argv[1], "r");
        if (file == NULL)
        {
            fprintf(stderr, "failed to open file '%s'\n", argv[1]);
            return EXIT_FAILURE;
        }
        should_file_close = true;
        break;
    }

    // Clean up

    if (should_file_close)
        fclose(file);

    return EXIT_SUCCESS;
}
