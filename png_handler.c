#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include "png_handler.h"


PNGImage load_png(const char *filename) {
    PNGImage img = {0};

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Error opening PNG file");
        return img;
    }

    unsigned char header[8];
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "Not a valid PNG file\n");
        fclose(fp);
        return img;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error reading PNG\n");
        fclose(fp);
        return img;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    img.width = png_get_image_width(png_ptr, info_ptr);
    img.height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    // ---- FIXED: Force conversion to RGBA ----
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    // Always ensure we have 4 channels (RGBA)
    if (!(color_type & PNG_COLOR_MASK_ALPHA))
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    img.pixels = (unsigned char *)malloc(rowbytes * img.height);
    png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * img.height);

    for (int y = 0; y < img.height; y++)
        row_pointers[y] = img.pixels + y * rowbytes;

    png_read_image(png_ptr, row_pointers);

    free(row_pointers);
    fclose(fp);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return img;
}
