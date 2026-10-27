#ifndef GZ_MENU_NAVIGATION_H
#define GZ_MENU_NAVIGATION_H

typedef unsigned int MenuNavU;

enum {
    MENU_NAV_WRAP = 1,
    MENU_NAV_CLAMP,
    MENU_NAV_INVENTORY,
    MENU_NAV_MAP,
    MENU_NAV_SHARDS
};

typedef struct {
    MenuNavU frame, direction, ticks, have_frame, context;
} MenuNavState;

typedef struct {
    MenuNavU x_bits, y_bits, frame, context;
    MenuNavU selection, count, policy, active;
    MenuNavU digital_navigation, action_edge;
} MenuNavInput;

/* Pure cursor/repeat metadata. No controller/action/native/save stores. */
MenuNavU _gz_mnu_step(MenuNavState *state, const MenuNavInput *input);
MenuNavU _gz_mnu_inventory_left(MenuNavU selection);
void _gz_mnu_tick(MenuNavU held, MenuNavU edges);

#endif
