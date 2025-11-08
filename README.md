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