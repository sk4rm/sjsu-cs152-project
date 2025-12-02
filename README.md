# vishellize

**vishellize** (portmanteau of "visualize" and "shell") is a command-line utility for rendering images as colorful text in your terminal.

Your terminal should support true 24-bit color and is using a monospace font (e.g. Consolas, JetBrains Mono, etc.).

## Usage

```bash
./vishellize --help
./vishellize image.jpg
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

You will need to install `libturbojpeg` (V3) with your package manager prior to compilation.

In Ubuntu, the default `libturbojpeg` version is v2. Refer to [the official libturbo-jpeg website](https://libjpeg-turbo.org/Downloads/YUM) for instructions on installing v3.

Then, compile with the following:

```bash
# Make
make

# Alternatively, manual
cc -Wall -Wextra \
	-I /opt/libjpeg-turbo/include \
	-L /opt/libjpeg-turbo/lib64 \
	-l turbojpeg \
	-l png \
	-o vishellize \
	png_handler.c main.c 
```

## Resources

https://www.compart.com/en/unicode/U+2584

https://libjpeg-turbo.org/Documentation/Documentation

https://stackoverflow.com/questions/59864651/how-to-use-the-utf-8-half-block-to-have-two-colors-in-one-character-block

https://en.wikipedia.org/wiki/List_of_file_signatures

https://github.com/libjpeg-turbo/libjpeg-turbo/blob/466c3448781cac35236b5e6770477a66d627b521/src/tjdecomp.c