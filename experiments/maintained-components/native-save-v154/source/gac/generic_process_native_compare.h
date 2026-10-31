#ifndef GZ_GENERIC_PROCESS_NATIVE_COMPARE_H
#define GZ_GENERIC_PROCESS_NATIVE_COMPARE_H
#include "owned_heap_save.h"

enum {
    GPN_NATIVE_INCOMPLETE = 2891,
    GPN_NATIVE_REFUSED = 2892
};

/* Exact fixed receiver for the real completed GHS producer. This is owned
 * source-backed BSS, never an image/stack/address supplied by a report. */
extern GhcSaveProducer _gz_gpn_completed_issuer;

/* Genuine GHS RELEASED callback hook, after OIS CURRENT publication. Only
 * PRIVATE classification runs.2891 means completed+fullyreleased diagnostic,
 *2892 means safelyreleased refusal;2886 retains uncertain storage ownership.
 * None of these is Native/Restore success. */
unsigned int _gz_gpn_current_compare(unsigned int request, unsigned int frame);
unsigned int _gz_gpn_native_private_apart(unsigned int pointer, unsigned int bytes);
extern volatile unsigned int _gz_gpn_native_report[32];
#endif
