#include "practice_selection.h"

static int same(const unsigned char *p, const char *text)
{
    /* Exact canonical strings, including their terminator, fit path[56]. */
    for (unsigned i = 0; i < 56; ++i) {
        if (p[i] != (unsigned char)text[i]) return 0;
        if (!text[i]) return 1;
    }
    return 0;
}

int gz_setup_select_practice(const unsigned char *rows, uint32_t count,
                             int choice, uint32_t *index)
{
    static const unsigned char title[8] = {0,5,0,0,0x10,0x57,0x5a,1};
    uint32_t found[2] = {0,0}, selected = UINT32_MAX;
    if (!index) return GZ_SETUP_SELECTION_INPUT;
    *index = UINT32_MAX;
    if ((choice != 0 && choice != 1) || count > 4096 || (!rows && count))
        return GZ_SETUP_SELECTION_INPUT;
    for (uint32_t i = 0; i < count; ++i) {
        const unsigned char *row = rows + i * 0x61u;
        unsigned j;
        for (j = 0; j < 8 && row[j] == title[j]; ++j) {}
        if (j != 8) continue;
        int device;
        if (same(row + 12, "/vol/storage_mlc01/usr/title/00050000/10575a01")) device = 0;
        else if (same(row + 12, "/vol/storage_usb01/usr/title/00050000/10575a01")) device = 1;
        else return GZ_SETUP_PRACTICE_PATH;
        if (++found[device] > 1) return GZ_SETUP_AMBIGUOUS_PRACTICE;
        if (device == choice) selected = i;
    }
    if (found[choice]) {
        *index = selected;
        return GZ_SETUP_SELECTED_PRACTICE;
    }
    return found[1 - choice] ? GZ_SETUP_OTHER_DEVICE : GZ_SETUP_NO_PRACTICE;
}
