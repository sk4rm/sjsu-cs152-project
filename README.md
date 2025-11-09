# vishellize

**vishellize** (portmanteau of "visualize" and "shell") is a command-line utility for rendering images as colorful text in your terminal.

Your terminal should support true 24-bit color and is using a monospace font (e.g. Consolas, JetBrains Mono, etc.).

## Usage

```bash
vishellize image.jpg
```

~~vishellize accepts input from stdin.~~

```bash
cat image.png | vishellize
```

## Build

### Windows

TurboJPEG static libraries for Windows have been provided in the repo. Just point your compiler to `libjpeg-turbo\include`, `libjpeg-turbo\lib`, and the library itself:

```powershell
# Example using Zig's drop-in C compiler
zig cc -Wall -Wextra `
    -I libjpeg-turbo/include `
    -L libjpeg-turbo/lib `
    -l turbojpeg `
    -o vishellize.exe `
    main.c
```

### Linux

You will need to install `libturbo-jpeg` with your package manager prior to compilation.

```bash
# Example using pacman and clang on Arch Linux
sudo pacman -S libjpeg-turbo
clang -Wall -Wextra \
    -I libjpeg-turbo/include \
    -L libjpeg-turbo/lib \
    -l turbojpeg \
    -o vishellize \
    main.c
```

## Resources

https://www.compart.com/en/unicode/U+2584

https://libjpeg-turbo.org/Documentation/Documentation

https://stackoverflow.com/questions/59864651/how-to-use-the-utf-8-half-block-to-have-two-colors-in-one-character-block

https://en.wikipedia.org/wiki/List_of_file_signatures

https://github.com/libjpeg-turbo/libjpeg-turbo/blob/466c3448781cac35236b5e6770477a66d627b521/src/tjdecomp.c