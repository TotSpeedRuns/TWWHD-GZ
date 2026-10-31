#ifndef GZ_JOINT_HEAP_LIFETIME_IMAGE_H
#define GZ_JOINT_HEAP_LIFETIME_IMAGE_H
#include "forest_snapshot.h"
/* Paired immutable source image, not a native writer-exclusion certificate.
 * The real held owner must freeze actors/allocators/resources before capture.
 * Stable observer counters prove metadata consistency only. This module does
 * not publish a user slot, restore the game, or grant ordinary admission. */
AgU32 _gz_aj_capture(const AfDomain*,void*staging,AgU32 capacity,AgU32*written);
AgU32 _gz_aj_validate(const AfDomain*,const void*image,AgU32 bytes);
#endif
