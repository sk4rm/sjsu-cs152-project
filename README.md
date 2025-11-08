# vishellize

**vishellize** (portmanteau of "visualize" and "shell") is a command-line utility for rendering images as colored text in your terminal.

## Usage

```bash
vishellize image.jpg
```

vishellize accepts input from stdin.

```bash
cat image.png | vishellize
```

## Build

Compile for Windows with the following flags:

```powershell
zig cc -Wall -Wextra `
    -I libjpeg-turbo/include `
    -L libjpeg-turbo/lib `
    -l turbojpeg `
    -o vishellize.exe `
    main.c
```

## Resources

https://www.compart.com/en/unicode/U+2584

https://libjpeg-turbo.org/Documentation/Documentation

https://stackoverflow.com/questions/59864651/how-to-use-the-utf-8-half-block-to-have-two-colors-in-one-character-block

https://en.wikipedia.org/wiki/List_of_file_signatures

https://github.com/libjpeg-turbo/libjpeg-turbo/blob/466c3448781cac35236b5e6770477a66d627b521/src/tjdecomp.c