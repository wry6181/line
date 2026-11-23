#include "arena.h"
#include "png.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



static void make_3d_path(const char *out_path, char *dst, size_t cap)
{
    size_t len = strlen(out_path);
    if (len > 4 && strcmp(out_path + len - 4, ".png") == 0) {
        snprintf(dst, cap, "%.*s_3d.png", (int)(len - 4), out_path);
    } else {
        snprintf(dst, cap, "%s_3d.png", out_path);
    }
}

int main(int argc, char **argv)
{
    const char *in_path = argc > 1 ? argv[1] : "test.png";
    const char *out_path = argc > 2 ? argv[2] : "out.png";

    MemArena *arena = arena_create(MByte(64));
    if (!arena) {
        fprintf(stderr, "out of memory\n");
        return EXIT_FAILURE;
    }

    PngImage img = load_png(arena, in_path);
    if (!img.pixels) {
        arena_destroy(arena);
        return EXIT_FAILURE;
    }



    if (!save_png(out_path, &img)) {
        arena_destroy(arena);
        return EXIT_FAILURE;
    }

    printf("%s: %ux%u, %u-bit %s%s, %llu bytes decoded as RGBA8\n",
           in_path, img.width, img.height, img.bit_depth,
           color_type_name(img.color_type),
           img.interlace_method ? ", interlaced" : "",
           (unsigned long long)img.size);

    arena_destroy(arena);
    return EXIT_SUCCESS;
}
