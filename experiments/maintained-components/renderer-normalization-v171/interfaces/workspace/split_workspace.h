#ifndef GZ_SPLIT_RESTORE_WORKSPACE_H
#define GZ_SPLIT_RESTORE_WORKSPACE_H
#include "mapped_ois_backend.h"
#include "owned_heap_held_view.h"
enum { GZ_SPLIT_WORK_LIMIT = 16u * 1024u * 1024u, GZ_SPLIT_PAGE = 65536u };
uint32_t _gz_ois_split_cut_header(void);
uint32_t _gz_ois_split_abandon(uint32_t result);
uint32_t _gz_ois_split_held_page(uint32_t, uint32_t, const OisHeldView *);
uint32_t _gz_ois_split_held_work(uint32_t, uint32_t, const OisHeldView *, uint32_t);
#endif
