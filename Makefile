CC ?= cc

TURBOJPEG_A = /opt/libjpeg-turbo/lib64/libturbojpeg.a
PNG_A       = /usr/lib/x86_64-linux-gnu/libpng.a
ZLIB_A      = /usr/lib/x86_64-linux-gnu/libz.a

CFLAGS = -Wall -Wextra -I/opt/libjpeg-turbo/include

# LDFLAGS = 
LDFLAGS = -static

# LDLIBS  = $(TURBOJPEG_A) $(PNG_A) $(ZLIB_A)
LDLIBS  = $(TURBOJPEG_A) $(PNG_A) $(ZLIB_A) -lm

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