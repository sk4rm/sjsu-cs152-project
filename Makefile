CC ?= cc

TARGET = vishellize
SRCS   = gif_handler.c png_handler.c main.c
OBJS   = $(SRCS:.c=.o)
CFLAGS = -Wall -Wextra

UNAME_S := $(shell uname -s)

# Linux-specific
ifeq ($(UNAME_S),Linux)
    # Include paths
    CFLAGS += -I/opt/libjpeg-turbo/include

    # Library locations
    TURBOJPEG_A = /opt/libjpeg-turbo/lib64/libturbojpeg.a
    PNG_A       = /usr/lib/x86_64-linux-gnu/libpng.a
    ZLIB_A      = /usr/lib/x86_64-linux-gnu/libz.a

    # Force static linking
    LDFLAGS += -static
    LDLIBS   = $(TURBOJPEG_A) $(PNG_A) $(ZLIB_A) -lm
endif

# macOS-specific
ifeq ($(UNAME_S),Darwin)
    # Include paths (including Apple Silicon Homebrew)
    CFLAGS += -I/opt/libjpeg-turbo/include -I/opt/homebrew/include

    # Library search paths
    LDFLAGS += -L/opt/libjpeg-turbo/lib64 -L/opt/homebrew/lib

    # Dynamic linking
    LDLIBS = -lturbojpeg -lpng -lz -lm
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean