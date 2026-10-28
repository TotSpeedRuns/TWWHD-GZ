/* Deliberately separate TU: one bounded reservation attempt, no retry/wait,
 * no SDK/native/heap/HL lock and no callback. Emitted RA must be literal zero. */
typedef unsigned int U;
U _gz_svd_try(volatile U *address)
{
    U old, acquired;
    __asm__ volatile(
        "li %1,0\n\t"
        "lwarx %0,0,%2\n\t"
        "cmpwi %0,0\n\t"
        "bne 1f\n\t"
        "li %0,1\n\t"
        "stwcx. %0,0,%2\n\t"
        "bne 1f\n\t"
        "isync\n\t"
        "li %1,1\n"
        "1:"
        : "=&r" (old), "=&r" (acquired) : "r" (address)
        : "cc", "memory");
    return acquired;
}
void _gz_svd_unlock(volatile U *address)
{
    __asm__ volatile("sync" ::: "memory");
    *address = 0;
}
