#include <stdint.h>
/* Called only with the live C++ destination-volume string borrowed by the
 * exact Native prepare caller and already consumed by its volume flush.
 * Length/pointer shape does not manufacture a physical object lifetime.
 */
int gz_setup_volume_choice(const unsigned char *text, uint32_t length)
{
    static const char mlc[] = "storage_mlc01";
    static const char usb[] = "storage_usb01";
    unsigned m = 1, u = 1;
    if (!text || length != 13) return -5;
    for (unsigned i = 0; i < 14; ++i) {
        if (text[i] != (unsigned char)mlc[i]) m = 0;
        if (text[i] != (unsigned char)usb[i]) u = 0;
    }
    return m ? 0 : u ? 1 : -5;
}
