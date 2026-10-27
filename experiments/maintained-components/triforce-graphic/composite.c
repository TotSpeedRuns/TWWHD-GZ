#include "composite.h"
#include <stddef.h>

/* Bounds also bound multiplication, row offsets and the per-draw work. */
#define TFG_LIMIT 2048u
#define TFG_Q16 65536

static uint32_t valid_descriptor(const void *descriptor, uint32_t bytes)
{
    uintptr_t address = (uintptr_t)descriptor;
    return descriptor && !(address & 3u) && address <= UINTPTR_MAX - bytes;
}

static uint32_t overlap(uintptr_t a, uint32_t a_bytes,
                        uintptr_t b, uint32_t b_bytes)
{
    /* Both pointer extents have already been checked for wrap. */
    return a < b + b_bytes && b < a + a_bytes;
}

static uint32_t valid_span(const void *pixels, uint32_t bytes,
                           uint32_t width, uint32_t height, uint32_t stride)
{
    uintptr_t address = (uintptr_t)pixels;
    uint32_t extent;

    if (!pixels || !width || !height || width > TFG_LIMIT ||
        height > TFG_LIMIT || stride < width * 4u || stride > 65536u)
        return 0;
    extent = (height - 1u) * stride + width * 4u;
    if (bytes < extent || bytes > 0x10000000u ||
        address > UINTPTR_MAX - bytes)
        return 0;
    return 1;
}

static uint32_t mul255(uint32_t a, uint32_t b)
{
    return (a * b + 127u) / 255u;
}

static uint32_t interpolate(uint32_t a, uint32_t b, uint32_t fraction)
{
    return (a * (256u - fraction) + b * fraction + 128u) >> 8;
}

static void sample(const struct tfg_sprite *source, int32_t u, int32_t v,
                   uint32_t linear, uint32_t rgba[4])
{
    uint32_t x = (uint32_t)u >> 16;
    uint32_t y = (uint32_t)v >> 16;
    const uint8_t *first = source->pixels + y * source->stride + x * 4u;
    uint32_t channel;

    if (!linear) {
        for (channel = 0; channel < 4; ++channel)
            rgba[channel] = first[channel];
    } else {
        uint32_t next_x = x + (x + 1u < source->width);
        uint32_t next_y = y + (y + 1u < source->height);
        const uint8_t *right = source->pixels + y * source->stride + next_x * 4u;
        const uint8_t *below = source->pixels + next_y * source->stride + x * 4u;
        const uint8_t *diagonal = source->pixels + next_y * source->stride + next_x * 4u;
        uint32_t fx = ((uint32_t)u >> 8) & 255u;
        uint32_t fy = ((uint32_t)v >> 8) & 255u;

        for (channel = 0; channel < 4; ++channel) {
            uint32_t top = interpolate(first[channel], right[channel], fx);
            uint32_t bottom = interpolate(below[channel], diagonal[channel], fx);
            rgba[channel] = interpolate(top, bottom, fy);
        }
    }
}

static void over(uint8_t *target, const uint32_t rgba[4], uint32_t tint)
{
    uint32_t source_alpha = mul255(rgba[3], tint & 255u);
    uint32_t target_alpha = target[3];
    uint32_t remaining = 255u - source_alpha;
    uint32_t alpha_denominator = source_alpha * 255u + target_alpha * remaining;
    uint32_t channel;

    if (!source_alpha)
        return;
    for (channel = 0; channel < 3; ++channel) {
        uint32_t color = mul255(rgba[channel], (tint >> (24u - channel * 8u)) & 255u);
        uint32_t numerator = color * source_alpha * 255u +
                             target[channel] * target_alpha * remaining;
        target[channel] = (uint8_t)((numerator + alpha_denominator / 2u) /
                                     alpha_denominator);
    }
    target[3] = (uint8_t)((alpha_denominator + 127u) / 255u);
}

static uint32_t mapped_corner(const struct tfg_quad *quad, int32_t x,
                              int32_t y, int32_t *u, int32_t *v)
{
    int64_t dx = (int64_t)x - quad->x;
    int64_t dy = (int64_t)y - quad->y;
    int64_t a = quad->u + dx * quad->du_dx + dy * quad->du_dy;
    int64_t b = quad->v + dx * quad->dv_dx + dy * quad->dv_dy;

    if (a < INT32_MIN || a > INT32_MAX || b < INT32_MIN || b > INT32_MAX)
        return 0;
    *u = (int32_t)a;
    *v = (int32_t)b;
    return 1;
}

uint32_t _gz_tfg_composite(struct tfg_plane *target,
                           const struct tfg_sprite *source,
                           const struct tfg_quad *quad)
{
    int32_t left, top, right, bottom, x, y;
    int32_t unused_u, unused_v;
    int64_t end_x, end_y;
    uintptr_t target_address, source_address;

    if (!valid_descriptor(target, sizeof(*target)) ||
        !valid_span(target->pixels, target->bytes, target->width,
                               target->height, target->stride))
        return TFG_BAD_PLANE;
    if (!valid_descriptor(source, sizeof(*source)) ||
        !valid_span(source->pixels, source->bytes, source->width,
                               source->height, source->stride))
        return TFG_BAD_SOURCE;
    if (!valid_descriptor(quad, sizeof(*quad)) ||
        !quad->width || !quad->height || quad->width > TFG_LIMIT ||
        quad->height > TFG_LIMIT || quad->filtering > 1u)
        return TFG_BAD_QUAD;

    target_address = (uintptr_t)target->pixels;
    source_address = (uintptr_t)source->pixels;
    if (overlap(target_address, target->bytes, source_address, source->bytes) ||
        overlap(target_address, target->bytes, (uintptr_t)target, sizeof(*target)) ||
        overlap(target_address, target->bytes, (uintptr_t)source, sizeof(*source)) ||
        overlap(target_address, target->bytes, (uintptr_t)quad, sizeof(*quad)))
        return TFG_OVERLAP;

    end_x = (int64_t)quad->x + quad->width;
    end_y = (int64_t)quad->y + quad->height;
    if (end_x > INT32_MAX || end_y > INT32_MAX)
        return TFG_BAD_QUAD;
    left = quad->x < 0 ? 0 : quad->x;
    top = quad->y < 0 ? 0 : quad->y;
    right = end_x > target->width ? (int32_t)target->width : (int32_t)end_x;
    bottom = end_y > target->height ? (int32_t)target->height : (int32_t)end_y;
    if (left >= right || top >= bottom)
        return TFG_OK;

    /* An affine coordinate's extrema occur at its rectangle's corners. */
    if (!mapped_corner(quad, left, top, &unused_u, &unused_v) ||
        !mapped_corner(quad, right - 1, top, &unused_u, &unused_v) ||
        !mapped_corner(quad, left, bottom - 1, &unused_u, &unused_v) ||
        !mapped_corner(quad, right - 1, bottom - 1, &unused_u, &unused_v))
        return TFG_BAD_QUAD;

    for (y = top; y < bottom; ++y) {
        int64_t dx = (int64_t)left - quad->x;
        int64_t dy = (int64_t)y - quad->y;
        int32_t u = (int32_t)(quad->u + dx * quad->du_dx + dy * quad->du_dy);
        int32_t v = (int32_t)(quad->v + dx * quad->dv_dx + dy * quad->dv_dy);
        uint8_t *pixel = target->pixels + (uint32_t)y * target->stride +
                         (uint32_t)left * 4u;
        for (x = left; x < right; ++x) {
            if (u >= 0 && v >= 0 &&
                (uint32_t)u < source->width * TFG_Q16 &&
                (uint32_t)v < source->height * TFG_Q16) {
                uint32_t rgba[4];
                sample(source, u, v, quad->filtering, rgba);
                over(pixel, rgba, quad->tint);
            }
            pixel += 4;
            /* Do not form an unused, possibly overflowing final coordinate. */
            if (x + 1 < right) {
                u += quad->du_dx;
                v += quad->dv_dx;
            }
        }
    }
    return TFG_OK;
}
