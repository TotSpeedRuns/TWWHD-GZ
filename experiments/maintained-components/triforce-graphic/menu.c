#include "composite.h"

struct tfg_piece {
    const uint8_t *pixels;
    const uint8_t *outline;
    uint32_t width;
    uint32_t height;
    int32_t native_x2;
    int32_t native_y2;
};

extern const struct tfg_piece _gz_tfg_pieces[8];
extern const struct tfg_piece _gz_tfg_full_piece;

/* Canonical sources, not acquisition history for the current save. */
static const char *const source_lines[8][2] = {
    {"Greatfish Isle (B4)", "Offshore salvage"},
    {"Private Oasis (E5)", "Cabana sewers"},
    {"Stone Watcher Island (C5)", "Offshore salvage"},
    {"Ghost Ship", "Changing Great Sea position"},
    {"Cliff Plateau Isles (G6)", "Offshore salvage"},
    {"Outset Island (B7)", "Savage Labyrinth, floor 30"},
    {"Stone Watcher Island (C5)", "Cave battle reward"},
    {"Overlook Island (G1)", "Cave battle reward"}
};

const char *_gz_tfg_source_line(uint32_t piece, uint32_t line)
{
    if (piece > 7u || line > 1u)
        return "";
    return source_lines[piece][line];
}

static void draw_piece(struct tfg_plane *canvas, const struct tfg_piece *piece,
                        int32_t center_x, int32_t center_y, uint32_t large,
                        uint32_t tint, uint32_t outline)
{
    int32_t width = (int32_t)piece->width + (outline ? 4 : 0);
    int32_t height = (int32_t)piece->height + (outline ? 4 : 0);
    int32_t x2 = piece->native_x2 - width;
    int32_t y2 = -piece->native_y2 - height;
    struct tfg_sprite sprite = {
        outline ? piece->outline : piece->pixels,
        (uint32_t)(width * height * 4),
        (uint32_t)width, (uint32_t)height, (uint32_t)width * 4u
    };
    struct tfg_quad quad = {
        center_x + (large ? x2 * 3 / 2 : x2 / 4),
        center_y + (large ? y2 * 3 / 2 : y2 / 4),
        large ? (uint32_t)width * 3u : ((uint32_t)width + 1u) / 2u,
        large ? (uint32_t)height * 3u : ((uint32_t)height + 1u) / 2u,
        0, 0, large ? 21845 : 131072, 0,
        0, large ? 21845 : 131072, tint, 1
    };
    (void)_gz_tfg_composite(canvas, &sprite, &quad);
}

/* Only the already-owned 1280x720 practice canvas is writable. Ownership and
 * cursor are scalars captured by the existing validated draw caller. No save,
 * actor, quest, chart, resource, native pane or game memory pointer is accepted.
 * Static native shard artwork; native material/shine reproduction is separate.
 */
void _gz_tfg_draw_parts(uint8_t *pixels, uint32_t ownership,
                        uint32_t selected, uint32_t large)
{
    struct tfg_plane canvas = {pixels, 1280u * 720u * 4u, 1280, 720, 5120};
    uint32_t piece;
    int32_t center_x = large ? 360 : 407;
    int32_t center_y = large ? 380 : 580;

    if (large > 1u)
        return;
    if ((ownership & 255u) == 255u) {
        /* The native full-image pane has the same default widget center.
         * Draw the authored selection boundary last so it remains visible.
         * Native demo, shine, material and pane state are not invoked here.
         */
        draw_piece(&canvas, &_gz_tfg_full_piece, center_x, center_y,
                   large, 0xffffffffu, 0);
        if (selected < 8u && large)
            draw_piece(&canvas, &_gz_tfg_pieces[selected], center_x, center_y,
                       large, 0xffffffffu, 1);
    } else {
        if (selected < 8u && large)
            draw_piece(&canvas, &_gz_tfg_pieces[selected], center_x, center_y,
                       large, 0xffffffffu, 1);
        for (piece = 0; piece < 8u; ++piece)
            draw_piece(&canvas, &_gz_tfg_pieces[piece], center_x, center_y, large,
                       ownership & (1u << piece) ? 0xffffffffu : 0x60606070u, 0);
    }
}
