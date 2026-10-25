#ifndef GZ_SAVE_ORIGIN_NATIVE_ADDRESSES_H
#define GZ_SAVE_ORIGIN_NATIVE_ADDRESSES_H
/* Fixed native objects/return PCs. Definitions live in native_aliases.ld.
 * Taking each symbol's address emits loader relocations; no object is defined
 * here and no native heap pointer, event tag or serialized mask is rebased. */
extern unsigned char _gz_spo_native_root_slot[];
extern unsigned char _gz_spo_native_clock_a[],_gz_spo_native_clock_b[],_gz_spo_native_clock_short[];
extern unsigned char _gz_spo_native_equipment[],_gz_spo_native_environment[],_gz_spo_native_equipped_items[];
extern unsigned char _gz_spo_boot_factory_return[],_gz_spo_boot_ctor_return[];
extern unsigned char _gz_spo_boot_initpre_return[],_gz_spo_boot_initpost_return[];
#define SPO_CLOCK_A ((SpbU32)_gz_spo_native_clock_a)
#define SPO_CLOCK_B ((SpbU32)_gz_spo_native_clock_b)
#define SPO_CLOCK_SHORT ((SpbU32)_gz_spo_native_clock_short)
#define SPO_EQUIPMENT ((SpbU32)_gz_spo_native_equipment)
#define SPO_ENVIRONMENT ((SpbU32)_gz_spo_native_environment)
#define SPO_EQUIPPED_ITEMS ((SpbU32)_gz_spo_native_equipped_items)
#endif
