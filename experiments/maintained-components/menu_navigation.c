/* Menu-local analog navigation. Gameplay stick/deadzone policy is untouched.
 * Only existing cursor metadata is changed; buttons and action queues are
 * never synthesized, edited, or invoked by this dispatcher.
 */
#include "menu_navigation.h"

typedef MenuNavU U;

enum {UP = 1, DOWN, LEFT, RIGHT};
#define PRESS_BITS 0x3f000000u
#define RELEASE_BITS 0x3eb33333u
#define ONE_BITS 0x3f800000u

extern volatile U _gz_state[], _gz_page, _gz_game_counter;
extern volatile U _gz_input_snapshot[], _gz_bind_masks[];
extern volatile U _gz_slots_mode, _gz_slots_selected, _gz_slots_notice;
extern volatile U _gz_rebind_mode, _gz_pending, _gz_life_phase, _gz_mht_phase;
extern volatile U _gz_list_count, _gz_map_sector;
extern const U _gz_map_quadrants[49][3], _gz_bag_descriptors[3][9];
extern U _gz_fav_navigation_count(void);
extern const char *_gz_btl_inventory_help(U row);

static MenuNavState mnu_state;

const char *_gz_mnu_inventory_help(U row)
{
    if (row == 27)
        return "Left: Grid    A: Shards";
    if (row == 26)
        return "Left: Grid L+Left: - Right: + A/X: Capacity Y: Refills";
    if (row == 25)
        return "Left: Grid  L+Left: -  Right/A: +  X: Piece";
    if (row == 24)
        return "Left: Grid    L+Left: -    Right/A: +";
    return _gz_btl_inventory_help(row);
}

static U mnu_held_chord(U held, U mask)
{
    return mask && (held & mask) == mask;
}

static U mnu_direction(U x_bits, U y_bits, U previous)
{
    U x = x_bits & 0x7fffffffu;
    U y = y_bits & 0x7fffffffu;
    U horizontal = x_bits >> 31 ? LEFT : RIGHT;
    U vertical = y_bits >> 31 ? DOWN : UP;

    if (x > ONE_BITS || y > ONE_BITS)
        return 0;
    if (x >= PRESS_BITS || y >= PRESS_BITS)
        return x > y ? horizontal : vertical;
    if ((previous == LEFT || previous == RIGHT) &&
        x >= RELEASE_BITS && previous == horizontal)
        return previous;
    if ((previous == UP || previous == DOWN) &&
        y >= RELEASE_BITS && previous == vertical)
        return previous;
    return 0;
}

U _gz_mnu_inventory_left(U selection)
{
    /* Existing drawn cursor Y values: grid214+90*r, panel230/360/490/580.
     * The nearest grid-row rightmost items are therefore5/17/23/23.
     */
    return selection == 24 ? 5 : selection == 25 ? 17 :
           selection == 26 || selection == 27 ? 23 : selection;
}

static U mnu_move(U selection, U count, U policy, U intent)
{
    if (selection >= count)
        return selection;
    if (policy == MENU_NAV_INVENTORY) {
        if (intent == LEFT && selection >= 24)
            return _gz_mnu_inventory_left(selection);
        if (intent == UP)
            return selection >= 25 ? selection - 1 : selection >= 6 ? selection - 6 : 0;
        if (intent == DOWN) {
            selection += selection >= 24 ? 1 : 6;
            return selection > 27 ? 27 : selection;
        }
        if (selection < 24) {
            if (intent == LEFT && selection)
                return selection - 1;
            if (intent == RIGHT)
                return selection + 1;
        }
        return selection;
    }
    if (policy == MENU_NAV_MAP) {
        if (intent == UP && selection >= 7)
            return selection - 7;
        if (intent == DOWN && selection < 42)
            return selection + 7;
        if (intent == LEFT && selection % 7)
            return selection - 1;
        if (intent == RIGHT && selection % 7 != 6)
            return selection + 1;
        return selection;
    }
    if (policy == MENU_NAV_SHARDS) {
        if (intent == LEFT)
            intent = UP;
        else if (intent == RIGHT)
            intent = DOWN;
    }
    if (intent == UP)
        return selection ? selection - 1 : policy == MENU_NAV_CLAMP ? 0 : count - 1;
    if (intent == DOWN)
        return selection + 1 < count ? selection + 1 : policy == MENU_NAV_CLAMP ? selection : 0;
    return selection;
}

U _gz_mnu_step(MenuNavState *state, const MenuNavInput *input)
{
    U intent;

    if (!state || !input)
        return 0;
    if (!input->active || !input->count || input->count > 803 ||
        input->policy < MENU_NAV_WRAP || input->policy > MENU_NAV_SHARDS) {
        state->direction = state->ticks = state->have_frame = 0;
        return input->selection;
    }
    /* Multiple native polls or a page change during one completed frame
     * cannot produce a second analog move. D-pad/action ownership wins even
     * if the new sample arrived after an analog poll in that same frame.
     */
    if (input->digital_navigation || input->action_edge) {
        state->direction = state->ticks = 0;
        state->frame = input->frame;
        state->have_frame = 1;
        return input->selection;
    }
    if (state->have_frame && state->frame == input->frame)
        return input->selection;
    if (!state->have_frame || input->frame - state->frame != 1u ||
        state->context != input->context || state->direction > RIGHT || state->ticks > 15) {
        state->direction = state->ticks = 0;
    }
    state->frame = input->frame;
    state->context = input->context;
    state->have_frame = 1;
    intent = mnu_direction(input->x_bits, input->y_bits, state->direction);
    if (!intent) {
        state->direction = state->ticks = 0;
        return input->selection;
    }
    if (intent != state->direction) {
        state->direction = intent;
        state->ticks = 0;
        return mnu_move(input->selection, input->count, input->policy, intent);
    }
    state->ticks = state->ticks < 12 ? state->ticks + 1 :
                   12 + ((state->ticks - 11u) & 3u);
    return state->ticks == 12 ? mnu_move(input->selection, input->count, input->policy, intent) : input->selection;
}

static U mnu_count(U page, U *policy)
{
    *policy = MENU_NAV_WRAP;
    if (page == 0 || page == 50)
        return 8;
    if (page >= 1 && page <= 5)
        return _gz_list_count;
    if (page == 6 || page == 60)
        return 10;
    if (page == 7)
        return 12;
    if (page == 8) {
        *policy = MENU_NAV_INVENTORY;
        return 28;
    }
    if (page >= 9 && page <= 11) {
        *policy = MENU_NAV_CLAMP;
        return _gz_bag_descriptors[page - 9][1];
    }
    if (page == 12)
        return 4;
    if (page == 13) {
        *policy = MENU_NAV_MAP;
        return 49;
    }
    if (page == 14)
        return _gz_map_sector < 49 ? _gz_map_quadrants[_gz_map_sector][2] : 0;
    if (page == 51)
        return 6;
    if (page == 52)
        return _gz_fav_navigation_count();
    if (page == 53)
        return 1;
    if (page == 54 || page == 58 || page == 59)
        return 2;
    if (page == 57) {
        *policy = MENU_NAV_SHARDS;
        return 8;
    }
    return 0;
}

void _gz_mnu_tick(U held, U edges)
{
    MenuNavInput input;
    U page = _gz_page;
    U modal = _gz_slots_mode;
    U selection, next;

    input.x_bits = _gz_input_snapshot[1];
    input.y_bits = _gz_input_snapshot[2];
    input.frame = _gz_game_counter;
    input.context = modal ? 0x100u + modal : page + 1u;
    input.active = (modal || _gz_state[0]) && !_gz_state[1] && !_gz_rebind_mode &&
                   !_gz_pending && !_gz_life_phase && !_gz_mht_phase;
    input.digital_navigation = (held & 0x0f00u) ||
        mnu_held_chord(held, _gz_bind_masks[3]) || mnu_held_chord(held, _gz_bind_masks[4]);
    input.action_edge = (edges & ~0x0f00u) != 0;
    if (modal) {
        input.count = modal <= 2 ? 10 : 0;
        input.policy = MENU_NAV_WRAP;
        selection = _gz_slots_selected;
    } else {
        input.count = mnu_count(page, &input.policy);
        selection = page == 13 ? _gz_map_sector : _gz_state[2];
    }
    input.selection = selection;
    next = _gz_mnu_step(&mnu_state, &input);
    if (next == selection)
        return;
    if (modal) {
        _gz_slots_selected = next;
        _gz_slots_notice = 0;
    } else if (page == 13) {
        _gz_map_sector = next;
    } else {
        _gz_state[2] = next;
    }
}
