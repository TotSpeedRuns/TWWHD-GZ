#ifndef GZ_RECOVERED_SLOT_LOCATION_H
#define GZ_RECOVERED_SLOT_LOCATION_H
typedef unsigned int SlotU;
typedef unsigned char SlotB;
typedef unsigned long long SlotLocationPair;
/* Borrowed saved descriptor. The caller must retain this real20B object and
 * all existing module metadata for the complete synchronous formatter call.
 * Numeric archive/header checks alone do not establish that lifetime.
 */
typedef struct {SlotU flags;SlotB stage[8];SlotU room_fields,extra;} SlotDescriptor;
typedef struct {const char *name;SlotU x,y;} SlotMapRow;
typedef struct {SlotU stage0,stage1;const char *name;} SlotNameRow;
_Static_assert(sizeof(SlotU)==4&&sizeof(void*)==4,"PPC32 ABI");
_Static_assert(sizeof(SlotDescriptor)==20,"saved descriptor ABI");
_Static_assert(sizeof(SlotMapRow)==12&&sizeof(SlotNameRow)==12,"retained metadata row ABI");
_Static_assert(sizeof(SlotLocationPair)==8,"r3 pointer/r4 room pair ABI");
/* Existing owners only. No definitions or new global storage are supplied. */
extern volatile SlotU _gz_slot_stage_buffer[3];
extern const volatile SlotMapRow _gz_map_quadrants[49];
extern const volatile SlotNameRow _gz_slot_location_table[];
extern const char _gz_slot_text_unknown[];
/* PPC EABI returns the high/low32-bit words in r3/r4 respectively. The high
 * word is a borrowed module string/scratch pointer; the low word is room0..255.
 * No returned pointer is an allocation lease. The next formatter call may
 * replace raw stage-buffer text, so callers must consume it synchronously.
 */
SlotLocationPair _gz_slot_location(const volatile SlotDescriptor *descriptor);
#endif
