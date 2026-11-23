#pragma once
#include "file.h"
#include "spng.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct PngImage PngImage;
struct PngImage {
    u8 *pixels;
    u64 size;
    u32 width;
    u32 height;
    u8 bit_depth;
    u8 color_type;
    u8 interlace_method;
};

static inline const char *color_type_name(u8 color_type)
{
    switch (color_type) {
    case 0: return "grayscale";
    case 2: return "truecolor";
    case 3: return "indexed";
    case 4: return "grayscale + alpha";
    case 6: return "truecolor + alpha";
    default: return "unknown";
    }
}

static inline PngImage load_png(MemArena *arena, const char *path)
{
    PngImage image = {0};

    BinFile file = read_file(arena, path);
    if (!file.data) {
        fprintf(stderr, "%s: could not read file\n", path);
        return image;
    }

    spng_ctx *ctx = spng_ctx_new(0);
    if (!ctx) {
        fprintf(stderr, "spng: out of memory\n");
        return image;
    }

    struct spng_ihdr ihdr = {0};
    int rc = spng_set_png_buffer(ctx, file.data, file.size);
    if (!rc) rc = spng_get_ihdr(ctx, &ihdr);

    size_t decoded_size = 0;
    if (!rc) rc = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &decoded_size);

    u8 *pixels = PUSH_ARRAY(arena, u8, decoded_size);
    if (!rc && !pixels) {
        fprintf(stderr, "%s: out of memory\n", path);
        spng_ctx_free(ctx);
        return image;
    }

    if (!rc) rc = spng_decode_image(ctx, pixels, decoded_size, SPNG_FMT_RGBA8, 0);

    spng_ctx_free(ctx);

    if (rc) {
        fprintf(stderr, "%s: %s\n", path, spng_strerror(rc));
        return image;
    }

    image.pixels = pixels;
    image.size = decoded_size;
    image.width = ihdr.width;
    image.height = ihdr.height;
    image.bit_depth = ihdr.bit_depth;
    image.color_type = ihdr.color_type;
    image.interlace_method = ihdr.interlace_method;
    return image;
}

static inline b32 save_png(const char *path, const PngImage *image)
{
    spng_ctx *ctx = spng_ctx_new(SPNG_CTX_ENCODER);
    if (!ctx) {
        fprintf(stderr, "spng: out of memory\n");
        return FALSE;
    }

    struct spng_ihdr ihdr = {
        .width = image->width,
        .height = image->height,
        .bit_depth = 8,
        .color_type = 6,
        .compression_method = 0,
        .filter_method = 0,
        .interlace_method = SPNG_INTERLACE_NONE,
    };

    int rc = spng_set_ihdr(ctx, &ihdr);
    if (!rc) rc = spng_set_option(ctx, SPNG_IMG_COMPRESSION_LEVEL, 6);

    FILE *out = NULL;
    if (!rc) {
        out = fopen(path, "wb");
        if (!out) {
            fprintf(stderr, "%s: %s\n", path, strerror(errno));
            spng_ctx_free(ctx);
            return FALSE;
        }
        rc = spng_set_png_file(ctx, out);
    }

    if (!rc) rc = spng_encode_image(ctx, image->pixels, image->size, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);

    if (out) fclose(out);
    spng_ctx_free(ctx);

    if (rc) {
        fprintf(stderr, "%s: %s\n", path, spng_strerror(rc));
        return FALSE;
    }
    return TRUE;
}
