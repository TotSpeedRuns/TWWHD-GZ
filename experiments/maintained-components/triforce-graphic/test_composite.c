#include "composite.h"

static void clear(uint8_t *p, unsigned size)
{
    unsigned i;
    for (i = 0; i < size; ++i)
        p[i] = 0;
}

static int equal(const uint8_t *a, const uint8_t *b, unsigned size)
{
    unsigned i;
    for (i = 0; i < size; ++i)
        if (a[i] != b[i])
            return 0;
    return 1;
}

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

int main(void)
{
    uint8_t pixels[64];
    uint8_t before[64];
    static const uint8_t source_pixels[16] = {
        255, 0, 0, 255, 0, 255, 0, 255,
        0, 0, 255, 255, 255, 255, 255, 128
    };
    static const uint8_t pink[4] = {255, 128, 128, 255};
    struct tfg_plane target = {pixels, sizeof(pixels), 4, 4, 16};
    struct tfg_sprite source = {source_pixels, sizeof(source_pixels), 2, 2, 8};
    struct tfg_quad quad = {1, 1, 2, 2, 0, 0, 65536, 0, 0, 65536, 0xffffffffu, 0};
    union {
        struct tfg_sprite descriptor;
        uint8_t backing[128];
    } aliased;
    unsigned i;

    clear(pixels, sizeof(pixels));
    CHECK(_gz_tfg_composite(&target, &source, &quad) == TFG_OK);
    CHECK(equal(pixels + 20, source_pixels, 8));
    CHECK(equal(pixels + 36, source_pixels + 8, 8));
    CHECK(pixels[0] == 0 && pixels[63] == 0);

    /* A half-alpha white pixel over opaque red has this hand-derived result. */
    quad.x = 1;
    quad.y = 1;
    quad.width = 1;
    quad.height = 1;
    quad.u = 65536;
    quad.v = 65536;
    CHECK(_gz_tfg_composite(&target, &source, &quad) == TFG_OK);
    CHECK(equal(pixels + 20, pink, 4));

    clear(pixels, sizeof(pixels));
    quad.x = -1;
    quad.y = -1;
    quad.width = 2;
    quad.height = 2;
    quad.u = 0;
    quad.v = 0;
    CHECK(_gz_tfg_composite(&target, &source, &quad) == TFG_OK);
    CHECK(equal(pixels, source_pixels + 12, 4));
    for (i = 4; i < sizeof(pixels); ++i)
        CHECK(pixels[i] == 0);

    for (i = 0; i < sizeof(before); ++i)
        before[i] = pixels[i];
    source.bytes = 15;
    CHECK(_gz_tfg_composite(&target, &source, &quad) == TFG_BAD_SOURCE);
    CHECK(equal(pixels, before, sizeof(pixels)));
    source.bytes = 16;
    source.pixels = pixels;
    CHECK(_gz_tfg_composite(&target, &source, &quad) == TFG_OVERLAP);
    CHECK(equal(pixels, before, sizeof(pixels)));

    /* Reproduces the reviewed hazard: output must not overwrite metadata that
     * later samples reread. The whole union is valid writable backing memory. */
    aliased.descriptor.pixels = source_pixels;
    aliased.descriptor.bytes = 16;
    aliased.descriptor.width = 2;
    aliased.descriptor.height = 2;
    aliased.descriptor.stride = 8;
    target.pixels = aliased.backing;
    target.bytes = sizeof(aliased.backing);
    CHECK(_gz_tfg_composite(&target, &aliased.descriptor, &quad) == TFG_OVERLAP);
    CHECK(aliased.descriptor.pixels == source_pixels && aliased.descriptor.bytes == 16);
    CHECK(_gz_tfg_composite(0, &source, &quad) == TFG_BAD_PLANE);
    return 0;
}
