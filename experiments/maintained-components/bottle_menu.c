/* Four native bottle slots; input queues intent, guarded game callback writes. */
#include "bottle_menu.h"

typedef BtlU U;
typedef unsigned char Byte;

extern volatile U _gz_state[], _gz_page, _gz_inv_request, _gz_bind_masks[];
extern volatile U _gz_heap_slot;
extern volatile Byte *volatile _gz_route_heap;
extern U _gz_route_heap_check(void);
extern void _gz_game_text(U, U, U, const char *);
extern const char _gz_inv_help[];

/* Exact EUR retail entries, authenticated and resolved by the link adapter. */
extern void _gz_btl_native_history(volatile Byte *flags, U content);
extern void _gz_btl_native_button(volatile Byte *items, U button, U content);
extern void _gz_btl_native_timer(volatile unsigned short *timer, U frames);
extern volatile Byte *_gz_btl_native_play(void);

#include "bottle_play.h"

static U selected_bottle;
static char current_line[64];
volatile U _gz_btl_report[16] = {0x42544c31u, 64, 1};

static const char *const titles[BTL_COUNT] = {
    "Bottle 1 Contents", "Bottle 2 Contents", "Bottle 3 Contents", "Bottle 4 Contents"
};
static const char *const contents[BTL_CONTENTS] = {
    "Empty Bottle", "Red Potion", "Green Potion", "Blue Potion", "Elixir Soup (Half)",
    "Elixir Soup (Full)", "Water", "Fairy", "Forest Firefly", "Forest Water"
};

static U match(U edges, U held, U mask)
{
    return mask && (edges & mask) && (held & mask) == mask;
}

static volatile Byte *current_save(void)
{
    volatile Byte *save;

    if (_gz_route_heap_check())
        return 0;
    save = _gz_route_heap;
    return (U)save == _gz_heap_slot + 0x20u ? save : 0;
}

static U queue(U request)
{
    if (_gz_inv_request)
        return 0;
    _gz_inv_request = request;
    return 1;
}

static U content_index(U value)
{
    return value >= BTL_FIRST_CONTENT && value < BTL_FIRST_CONTENT + BTL_CONTENTS ?
        value - BTL_FIRST_CONTENT : 0;
}

static const char *content_name(U value)
{
    if (value == BTL_NONE)
        return "No Bottle";
    return value >= BTL_FIRST_CONTENT && value < BTL_FIRST_CONTENT + BTL_CONTENTS ?
        contents[value - BTL_FIRST_CONTENT] : "Unknown Contents";
}

U _gz_btl_inventory_input(U edges, U held)
{
    U row = _gz_state[2];
    volatile Byte *save;

    if (!_gz_state[0] || _gz_page != 8 || row < BTL_FIRST_ROW || row - BTL_FIRST_ROW >= BTL_COUNT)
        return 0;
    if (edges & 0x2000u) {
        queue(BTL_TOGGLE_REQUEST + row - BTL_FIRST_ROW);
        return 1;
    }
    if (!match(edges, held, _gz_bind_masks[5]))
        return 0;
    selected_bottle = row - BTL_FIRST_ROW;
    save = current_save();
    _gz_state[2] = save ? content_index(save[0x4a + selected_bottle]) : 0;
    _gz_page = BTL_PAGE;
    return 1;
}

void _gz_btl_contents_input(U edges, U held, U repeat)
{
    U row;

    if (!_gz_state[0] || _gz_page != BTL_PAGE)
        return;
    if (match(edges, held, _gz_bind_masks[6])) {
        _gz_page = 8;
        _gz_state[2] = BTL_FIRST_ROW + (selected_bottle < BTL_COUNT ? selected_bottle : 0);
        return;
    }
    if (selected_bottle >= BTL_COUNT)
        return;
    row = _gz_state[2];
    if (row >= BTL_CONTENTS)
        row = 0;
    if (match(edges | repeat, held, _gz_bind_masks[3]))
        row = row ? row - 1 : BTL_CONTENTS - 1;
    if (match(edges | repeat, held, _gz_bind_masks[4]))
        row = row + 1 < BTL_CONTENTS ? row + 1 : 0;
    _gz_state[2] = row;
    if (match(edges, held, _gz_bind_masks[5]))
        queue(BTL_CONTENT_REQUEST + selected_bottle * BTL_CONTENTS + row);
}

const char *_gz_btl_inventory_help(U row)
{
    return row >= BTL_FIRST_ROW && row - BTL_FIRST_ROW < BTL_COUNT ?
        "A: Contents   X: Give/Delete Bottle   Y: Refills" : _gz_inv_help;
}

static void current_text(const char *name)
{
    U i = 0, j = 0;
    const char prefix[] = "Current: ";

    while (prefix[i] && i + 1 < sizeof(current_line)) {
        current_line[i] = prefix[i];
        i++;
    }
    while (name[j] && i + 1 < sizeof(current_line))
        current_line[i++] = name[j++];
    current_line[i] = 0;
}

void _gz_btl_draw(void)
{
    U i, row;
    volatile Byte *save;

    if (_gz_page != BTL_PAGE || selected_bottle >= BTL_COUNT)
        return;
    _gz_game_text(0, 2, 1, titles[selected_bottle]);
    save = current_save();
    current_text(save ? content_name(save[0x4a + selected_bottle]) : "Unavailable");
    _gz_game_text(0, 4, 2, current_line);
    row = _gz_state[2];
    if (row >= BTL_CONTENTS)
        row = 0;
    for (i = 0; i < BTL_CONTENTS; i++) {
        _gz_game_text(0, 4, 4 + i, contents[i]);
        if (i == row)
            _gz_game_text(0, 2, 4 + i, ">");
    }
    _gz_game_text(0, 4, 18, "A: Choose Contents   B: Inventory");
}

static U flags_value(volatile Byte *save)
{
    return ((U)save[0x51] << 16) | ((U)save[0x52] << 8) | save[0x53];
}

/* Called only from the unchanged ROH/live/current-save inventory apply path.
 * Recheck the exact current root before touching any save or native binding. */
void _gz_btl_apply(U request, volatile Byte *save)
{
    U bottle, content, slot, old, button, mask = 0;
    volatile Byte *play = 0;
    BtlPlayRoots play_roots;

    _gz_btl_report[3]++;
    _gz_btl_report[4] = request;
    _gz_btl_report[11] = 1;
    if (request >= BTL_CONTENT_REQUEST && request < BTL_CONTENT_REQUEST + BTL_COUNT * BTL_CONTENTS) {
        U value = request - BTL_CONTENT_REQUEST;
        bottle = value / BTL_CONTENTS;
        content = BTL_FIRST_CONTENT + value % BTL_CONTENTS;
    } else if (request >= BTL_TOGGLE_REQUEST && request < BTL_TOGGLE_REQUEST + BTL_COUNT) {
        bottle = request - BTL_TOGGLE_REQUEST;
        content = BTL_NONE;
    } else
        return;
    if (!save || (U)save != _gz_heap_slot + 0x20u || save != _gz_route_heap) {
        _gz_btl_report[11] = 2;
        return;
    }
    slot = BTL_FIRST_SLOT + bottle;
    old = save[0x3c + slot];
    if (request >= BTL_TOGGLE_REQUEST)
        content = old == BTL_NONE ? BTL_FIRST_CONTENT : BTL_NONE;
    if (content == BTL_FOREST_WATER) {
        if (!btl_play_roots(&play_roots)) {
            _gz_btl_report[11] = 3;
            return;
        }
        play = _gz_btl_native_play();
        if (!btl_save_current(save) || !btl_play_current(play, &play_roots)) {
            _gz_btl_report[11] = 3;
            return;
        }
    }
    for (button = 0; button < 3; button++)
        if (save[9 + button] == slot)
            mask |= 1u << button;
    _gz_btl_report[5] = bottle;
    _gz_btl_report[6] = old;
    _gz_btl_report[7] = content;
    _gz_btl_report[8] = mask;
    _gz_btl_report[9] = *(volatile unsigned short *)(save + 0x66);
    _gz_btl_report[12] = flags_value(save);
    if (content != BTL_NONE) {
        _gz_btl_native_history(save + 0x51, BTL_FIRST_CONTENT);
        if (content != BTL_FIRST_CONTENT)
            _gz_btl_native_history(save + 0x51, content);
    }
    /* Retail fresh filling initializes one shared timer, not a per-bottle timer.
     * Other edits/removals deliberately retain the native singleton unchanged. */
    if (content == BTL_FOREST_WATER)
        _gz_btl_native_timer((volatile unsigned short *)(save + 0x66), 54000);
    if (!mask)
        save[0x3c + slot] = (Byte)content;
    else
        for (button = 0; button < 3; button++)
            if (mask & (1u << button))
                _gz_btl_native_button(save + 0x3c, button, content);
    /* Retail meter initialization activates a positive saved timer. A direct
     * editor fill needs that transition after reset, without invoking a Link
     * or meter initializer. Native pause/update/expiry gates remain in charge.
     */
    if (content == BTL_FOREST_WATER &&
        !btl_start_positive_timer(save, play, &play_roots)) {
        _gz_btl_report[11] = 3;
        return;
    }
    _gz_btl_report[10] = *(volatile unsigned short *)(save + 0x66);
    _gz_btl_report[13] = flags_value(save);
    _gz_btl_report[11] = 0;
    _gz_btl_report[14]++;
}
