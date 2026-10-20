#ifndef GZ_B294_TYPED_CORE_H
#define GZ_B294_TYPED_CORE_H
/* Private inventory-bomb component. Main-slot registration is separate. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
enum { PB_ROW_BYTES=2048, PB_FIELD_BYTES=1024, PB_MAGIC=0x50423134 };
typedef struct {u16 offset,count;u8 width,finite;u16 reserved;} PbField;
typedef struct {u32 stage0,stage1,room,layer,generation,frame;} PbContext;
typedef struct {
 u32 valid,magic,bytes,kind,pid,actor,parameters,parent;
 u32 resources[6];
 u8 home[20];
 PbContext context;
 u32 scalar_bytes;
 u8 fields[PB_FIELD_BYTES];
 u8 reserved[920];
} PbRow;
typedef char PbSize[(sizeof(PbRow)==PB_ROW_BYTES)?1:-1];
extern const PbField _gz_b294_core_fields[];
extern const u32 _gz_b294_core_field_count;
/* Pure preflight; no writes to game actors, effects or registries. */
u32 _gz_b294_core_supported(void* actor);
u32 _gz_b294_core_capture(void* actor,PbRow* row,const PbContext* context);
u32 _gz_b294_core_record_safe(const PbRow* row);
/* Root must validate the entire restore transaction before invoking apply. */
/* Pure recipe; no saved pointer is dereferenced or published as an actor. */
enum { PB_CAPACITY=8, PB_BANK_BYTES=32+PB_CAPACITY*PB_ROW_BYTES };
typedef struct {u32 valid,magic,count,bytes,reserved[4];PbRow rows[PB_CAPACITY];} PbBank;
typedef char PbBankSize[(sizeof(PbBank)==PB_BANK_BYTES)?1:-1];
u32 _gz_b294_core_bank_capture(PbBank*,u32 queue,const PbContext*);
u32 _gz_b294_core_bank_record_safe(const PbBank*);

#endif
