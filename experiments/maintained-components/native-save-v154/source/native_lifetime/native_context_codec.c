#include "native_context_cut.h"
#include <stdint.h>

NgcU _gz_ngc_checksum(const void *image, NgcU bytes) {
    const unsigned char *data = image;
    NgcU i, hash = 2166136261u;
    if (!image || bytes != NGC_BYTES) return 0;
    for (i = 0; i < bytes; ++i) {
        NgcU byte = ((i >= 12 && i < 16) || (i >= 96 && i < 100)) ? 0 : data[i];
        hash = (hash ^ byte) * 16777619u;
    }
    return hash;
}

NgcU _gz_ngc_validate(const void *image, NgcU bytes) {
    const NgcHeader *header = image;
    NgcU i;
    if (!image || ((uintptr_t)image & 3u) || bytes != NGC_BYTES ||
        header->magic != NGC_MAGIC || header->bytes != NGC_BYTES ||
        header->version != NGC_VERSION || header->valid != 1 || header->missing != 255 ||
        header->address != NGC_ADDRESS || header->data != NGC_DATA ||
        header->payload_bytes != NGC_PAYLOAD_BYTES || header->flags != NGC_ROOTS_ONLY ||
        header->initialized != 1 || header->producer.version != 2 ||
        !ghs_provenance_format(&header->producer, header->producer.request,
            header->producer.frame, header->producer.hg, header->producer.lease,
            header->producer.original_return, header->producer.thread)) return NGC_FORMAT;
    for (i = 0; i < 5; ++i) if (header->reserved[i]) return NGC_FORMAT;
    for (i = NGC_DATA + NGC_PAYLOAD_BYTES; i < NGC_BYTES; ++i)
        if (((const unsigned char *)image)[i]) return NGC_FORMAT;
    return header->checksum == _gz_ngc_checksum(image, bytes) ? NGC_OK : NGC_FORMAT;
}
