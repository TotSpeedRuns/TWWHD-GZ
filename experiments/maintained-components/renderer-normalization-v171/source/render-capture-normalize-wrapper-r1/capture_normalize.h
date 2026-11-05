#ifndef GZ_CAPTURE_NORMALIZE_H
#define GZ_CAPTURE_NORMALIZE_H
#include <stdint.h>
typedef uint32_t RcwU;
RcwU _gz_rcw_capture(RcwU request,RcwU frame,RcwU pointer,RcwU capacity,RcwU *written);
RcwU _gz_rcl_normalized_scope(RcwU request,RcwU frame);
#endif
