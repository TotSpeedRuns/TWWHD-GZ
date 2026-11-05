#ifndef GZ_SETUP_PRACTICE_SELECTION_H
#define GZ_SETUP_PRACTICE_SELECTION_H
#include <stdint.h>
enum gz_setup_selection {
    GZ_SETUP_NO_PRACTICE = 0,
    GZ_SETUP_SELECTED_PRACTICE = 1,
    GZ_SETUP_SELECTION_INPUT = -1,
    GZ_SETUP_OTHER_DEVICE = -2,
    GZ_SETUP_AMBIGUOUS_PRACTICE = -3,
    GZ_SETUP_PRACTICE_PATH = -4
};
/* rows is the complete successful MCP_TitleList result, count <= capacity.
 * Each packed MCPTitleListType is exactly 0x61 bytes, with BE titleId at 0
 * and path[56] at 0x0c. Caller owns this unchanged extent and a separate index.
 * choice 0 = MLC, 1 = USB01. This function grants no filesystem permission.
 * On any result except SELECTED, index is UINT32_MAX. Only NO_PRACTICE may
 * enter new-install logic; OTHER_DEVICE never authorizes a second install.
 */
int gz_setup_select_practice(const unsigned char *rows, uint32_t count,
                             int choice, uint32_t *index);
#endif
