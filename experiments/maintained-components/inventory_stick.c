/* Inventory page 8 only. Raw canonical stick bits are read after _gz_read_input.
 * Analog intent changes the cursor, never synthesized button/action bits.
 * In particular, lower-row Left/Right retain their D-pad-only value editing.
 */
#include "inventory_stick.h"

typedef InvStickU U;

extern volatile U _gz_state[], _gz_page, _gz_game_counter;
extern volatile U _gz_input_snapshot[], _gz_bind_masks[];

enum {
    INV_PAGE = 8,
    INV_LAST_ROW = 27,
    INV_GRID_ITEMS = 24,
    INV_COLUMNS = 6,
    INV_UP = 1,
    INV_DOWN = 2,
    INV_LEFT = 3,
    INV_RIGHT = 4,
    INV_INITIAL_REPEAT = 12
};

/* Positive IEEE-754 normalized f32 magnitudes are ordered as unsigned words.
 * Keep this menu-local threshold separate from any gameplay deadzone tool.
 */
#define INV_PRESS_BITS 0x3f000000u
#define INV_RELEASE_BITS 0x3eb33333u
#define INV_ONE_BITS 0x3f800000u

static InvStickState stick_state;

static U direction(U x_bits, U y_bits, U previous)
{
    U x = x_bits & 0x7fffffffu;
    U y = y_bits & 0x7fffffffu;
    U horizontal = x_bits >> 31 ? INV_LEFT : INV_RIGHT;
    U vertical = y_bits >> 31 ? INV_DOWN : INV_UP;

    /* Invalid, infinite and NaN samples cannot become navigation. */
    if (x > INV_ONE_BITS || y > INV_ONE_BITS)
        return 0;
    if (x >= INV_PRESS_BITS || y >= INV_PRESS_BITS)
        return x > y ? horizontal : vertical;
    if ((previous == INV_LEFT || previous == INV_RIGHT) &&
        x >= INV_RELEASE_BITS && previous == horizontal)
        return previous;
    if ((previous == INV_UP || previous == INV_DOWN) &&
        y >= INV_RELEASE_BITS && previous == vertical)
        return previous;
    return 0;
}

static U move_cursor(U selection, U intent)
{
    if (selection > INV_LAST_ROW)
        return selection;
    if (intent == INV_UP)
        selection = selection >= 25 ? selection - 1 :
                    selection >= INV_COLUMNS ? selection - INV_COLUMNS : 0;
    else if (intent == INV_DOWN)
        selection += selection >= INV_GRID_ITEMS ? 1 : INV_COLUMNS;
    else if (selection < INV_GRID_ITEMS) {
        if (intent == INV_LEFT && selection)
            selection--;
        else if (intent == INV_RIGHT)
            selection++;
    }
    return selection > INV_LAST_ROW ? INV_LAST_ROW : selection;
}

U _gz_ins_step(InvStickState *state, U x_bits, U y_bits, U digital_navigation,
               U frame, U selection, U active)
{
    U intent;

    if (!state)
        return selection;
    if (!active) {
        state->direction = 0;
        state->ticks = 0;
        state->have_frame = 0;
        return selection;
    }
    /* Several native input polls in one completed frame may not move twice. */
    if (state->have_frame && state->frame == frame)
        return selection;
    if (!state->have_frame || frame - state->frame != 1u ||
        state->direction > INV_RIGHT || state->ticks > 15u) {
        state->direction = 0;
        state->ticks = 0;
    }
    state->frame = frame;
    state->have_frame = 1;
    if (digital_navigation) {
        state->direction = 0;
        state->ticks = 0;
        return selection;
    }
    intent = direction(x_bits, y_bits, state->direction);
    if (!intent) {
        state->direction = 0;
        state->ticks = 0;
        return selection;
    }
    if (intent != state->direction) {
        state->direction = intent;
        state->ticks = 0;
        return move_cursor(selection, intent);
    }
    /* First repeat after 12 completed frames, then one every four frames.
     * The bounded counter cycles 12..15 rather than growing indefinitely.
     */
    state->ticks = state->ticks < INV_INITIAL_REPEAT ? state->ticks + 1 :
                   INV_INITIAL_REPEAT + ((state->ticks - 11u) & 3u);
    return state->ticks == INV_INITIAL_REPEAT ? move_cursor(selection, intent) : selection;
}

static U held_chord(U held, U mask)
{
    return mask && (held & mask) == mask;
}

U _gz_ins_inventory(U selection, U held)
{
    U active = _gz_state[0] && _gz_page == INV_PAGE;
    U digital = (held & 0x0f00u) || held_chord(held, _gz_bind_masks[3]) ||
                held_chord(held, _gz_bind_masks[4]);
    U x_bits = _gz_input_snapshot[1];
    U y_bits = _gz_input_snapshot[2];

    return _gz_ins_step(&stick_state, x_bits, y_bits, digital,
                        _gz_game_counter, selection, active);
}
