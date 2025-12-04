CC ?= cc

# Homebrew library locations (Apple Silicon)
TURBOJPEG_A = /opt/libjpeg-turbo/lib64/libturbojpeg.a
PNG_LIBPATH = /opt/homebrew/lib
ZLIB_LIBPATH = /opt/homebrew/lib

# Include paths
CFLAGS = -Wall -Wextra -I/opt/libjpeg-turbo/include -I/opt/homebrew/include

# macOS cannot use -static, so remove it
LDFLAGS = -L/opt/libjpeg-turbo/lib64 -L/opt/homebrew/lib

# Link dynamically on macOS
LDLIBS = -lturbojpeg -lpng -lz -lm

TARGET = vishellize
SRCS   = png_handler.c main.c
OBJS   = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
