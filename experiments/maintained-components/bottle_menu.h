#ifndef GZ_BOTTLE_MENU_H
#define GZ_BOTTLE_MENU_H

typedef unsigned int BtlU;

enum {
    BTL_PAGE = 60,
    BTL_COUNT = 4,
    BTL_CONTENTS = 10,
    BTL_FIRST_ROW = 15,
    BTL_FIRST_SLOT = 14,
    BTL_FIRST_CONTENT = 0x50,
    BTL_FOREST_WATER = 0x59,
    BTL_NONE = 0xff,
    BTL_CONTENT_REQUEST = 960,
    BTL_TOGGLE_REQUEST = 1000
};

extern volatile BtlU _gz_btl_report[16];
BtlU _gz_btl_inventory_input(BtlU edges, BtlU held);
void _gz_btl_contents_input(BtlU edges, BtlU held, BtlU repeat);
void _gz_btl_draw(void);
const char *_gz_btl_inventory_help(BtlU row);
void _gz_btl_apply(BtlU request, volatile unsigned char *save);

#endif
