#ifndef GZ_NATIVE_CONTEXT_CUT_H
#define GZ_NATIVE_CONTEXT_CUT_H

#include "owned_heap_save.h"

typedef unsigned int NgcU;
enum {
    NGC_MAGIC = 0x4e474331u, NGC_VERSION = 1,
    NGC_ADDRESS = 0x1046f0b0u, NGC_PAYLOAD_BYTES = 0x62f8u,
    NGC_INITIALIZED_ADDRESS = 0x104753a8u,
    NGC_DATA = 128, NGC_BYTES = 25472, NGC_ROOTS_ONLY = 1,
    NGC_OK = 0, NGC_ARGUMENT = 1, NGC_SCOPE = 2,
    NGC_SPACE = 3, NGC_CHANGED = 4, NGC_FORMAT = 5, NGC_PROTECTED = 6
};

typedef struct {
    NgcU magic, bytes, version, valid, missing, address, data, payload_bytes;
    GhcSaveProducer producer;
    NgcU checksum, flags, initialized, reserved[5];
} NgcHeader;

typedef char NgcHeader128[sizeof(NgcHeader) == NGC_DATA ? 1 : -1];
typedef char NgcGuardExcluded[NGC_ADDRESS + NGC_PAYLOAD_BYTES == NGC_INITIALIZED_ADDRESS ? 1 : -1];

/* Address is an immutable format key, not a pointer selected by a packet.
 * Native access binds separate original ELF DATA aliases. Pointer values
 * inside this opaque CPU object are never followed by these routines.
 */
NgcU _gz_ngc_checksum(const void *image, NgcU bytes);
NgcU _gz_ngc_validate(const void *image, NgcU bytes);
NgcU _gz_ngc_capture(NgcU request, NgcU frame, void *output, NgcU capacity);
NgcU _gz_ngc_verify_held(NgcU request, NgcU frame, const void *image, NgcU bytes);
NgcU _gz_ngc_held_step(NgcU request, NgcU frame, NgcU phase, NgcU write);

#endif
