/* Keep this C translation unit separate, without LTO. The caller bounds the
 * report index and materializes the complete address before this call. */
#include <stdint.h>

void _gz_jwa_increment_counter(volatile uint32_t *counter)
{
    __sync_fetch_and_add(counter, 1);
}
