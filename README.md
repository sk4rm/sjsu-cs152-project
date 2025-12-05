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

### Prerequisites

1. libturbojpeg.a>=3.0
2. libpng.a
3. libz.a (typically included with libpng)

### Linux

You will need to install `libturbojpeg` (V3) with your package manager prior to compilation.

In Ubuntu, the default `libturbojpeg` version is v2. Refer to [the official libturbo-jpeg website](https://libjpeg-turbo.org/Downloads/YUM) for instructions on installing v3.

Then, compile with the following:

```bash
make
```

## Resources

https://www.compart.com/en/unicode/U+2584

https://libjpeg-turbo.org/Documentation/Documentation

https://stackoverflow.com/questions/59864651/how-to-use-the-utf-8-half-block-to-have-two-colors-in-one-character-block

https://en.wikipedia.org/wiki/List_of_file_signatures

https://github.com/libjpeg-turbo/libjpeg-turbo/blob/466c3448781cac35236b5e6770477a66d627b521/src/tjdecomp.c

https://github.com/nothings/stb/blob/master/stb_image.h  
stb_image.h — public-domain / MIT-licensed single-header library for image decoding  
Used for GIF decoding and multi-frame extraction.
