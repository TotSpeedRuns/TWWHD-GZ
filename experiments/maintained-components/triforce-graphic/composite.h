#ifndef GZ_TRIFORCE_COMPOSITE_H
#define GZ_TRIFORCE_COMPOSITE_H

#include <stdint.h>

/* CPU-owned straight RGBA8 planes. No native UI, resource or GPU pointers. */
struct tfg_plane {
    uint8_t *pixels;
    uint32_t bytes;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
};

struct tfg_sprite {
    const uint8_t *pixels;
    uint32_t bytes;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
};

/* Destination rectangle, followed by destination-to-source Q16.16 mapping.
 * (u,v) is the source coordinate of destination (x,y). The four increments
 * permit flipped/scaled/rotated quads. Out-of-source samples are transparent.
 * Tint is RGBA, with the R byte most significant on either host byte order.
 * Filtering: 0 nearest; 1 bilinear, clamped within the source plane.
 */
struct tfg_quad {
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
    int32_t u;
    int32_t v;
    int32_t du_dx;
    int32_t dv_dx;
    int32_t du_dy;
    int32_t dv_dy;
    uint32_t tint;
    uint32_t filtering;
};

enum tfg_result {
    TFG_OK = 0,
    TFG_BAD_PLANE = 1,
    TFG_BAD_SOURCE = 2,
    TFG_BAD_QUAD = 3,
    TFG_OVERLAP = 4
};

/* The caller owns valid, stable, readable metadata descriptors and readable
 * source / writable target memory. This API checks extents, not OS mappings.
 * Validate every span and transformed corner before the first pixel store.
 * Only the clipped destination pixels can change. Invalid input is atomic.
 * The destination cannot overlap source pixels or any input descriptor.
 * No allocation/callbacks.
 * This is an authored source-over compositor, not a claim that native NW4F
 * material combiners, indirect noise or shine animations have been reproduced.
 */
uint32_t _gz_tfg_composite(struct tfg_plane *target,
                           const struct tfg_sprite *source,
                           const struct tfg_quad *quad);

#endif
