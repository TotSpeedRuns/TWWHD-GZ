/* Observed native JPA births/removals. A serial is an event identity, NOT
 * allocation ownership, a particle freeze, or permission to replay an effect. */
typedef unsigned int u32;
enum { JEL_MANAGERS=8, JEL_ROWS=256, JEL_CHILDREN=64 };
enum { JEL_LOST=1, JEL_LIMIT=2, JEL_WRAP=4, JEL_BAD=8, JEL_UNSEEN=16 };
typedef struct { u32 pointer,epoch,capacity; } JelManager;
typedef struct {
 u32 manager,manager_epoch,emitter,serial,resource,group,index,alive;
} JelRow;
typedef struct { u32 pointer,epoch,alive,reserved,emitter[2],generation[2],manager[2]; } JelChild;
typedef struct {
 u32 magic,bytes,version,enabled,busy,flags,serial,events;
 JelManager manager[JEL_MANAGERS]; JelRow row[JEL_ROWS]; JelChild child[JEL_CHILDREN];
} JelState;
extern JelState _gz_jel_state;
void _gz_jel_manager(u32,u32);
void _gz_jel_birth(u32,u32,u32,u32,u32);
void _gz_jel_remove(u32,u32);
u32 _gz_jel_lookup(u32,u32);
u32 _gz_jel_match(u32,u32,u32,u32,u32,u32);
void _gz_jel_child_born(u32);
void _gz_jel_child_ending(u32);
void _gz_jel_edge(u32,u32,u32);
u32 _gz_jel_child_epoch(u32);
u32 _gz_jel_edge_generation(u32,u32,u32,u32);

/* Constructor receipts describe the native array layout. They are not heap
 * allocation-generation certificates and do not exclude particle writers. */
typedef struct {
 u32 manager,construction,base,capacity,serial,epoch,phase,reserved;
} JelPool;
extern JelPool _gz_jel_pools[JEL_MANAGERS];
void _gz_jel_constructing(u32 manager);
void _gz_jel_pool(u32 manager,u32 first_emitter,u32 count);
u32 _gz_jel_pool_match(u32 manager,u32 emitter,u32 generation);

#ifndef GZ_JEL_AUXILIARY_POOL_LAYOUT_H
#define GZ_JEL_AUXILIARY_POOL_LAYOUT_H
/* Constructor-layout metadata only. A completed receipt is not a heap
 * allocation-generation certificate, producer freeze or replay permission.
 * Native stride is fixed by owned constructor028216A4 / element0282301C.
 * phase2 count0/base0 is an absent zero-element array. A non-null count0
 * array retains its8-byte cookie (product1, complementFFFFFFFF). */
enum { JEL_AUX_STRIDE=0x9c, JEL_AUX_COOKIE=8,
 JEL_AUX_EMPTY=0, JEL_AUX_CONSTRUCTING=1, JEL_AUX_COMPLETE=2,
 JEL_AUX_UNPROVED=3 };
typedef struct {
 u32 manager,construction,base,count,serial,epoch,phase,reserved;
} JelAuxPool;
typedef char JelAuxPool32[sizeof(JelAuxPool)==32?1:-1];
extern JelAuxPool _gz_jel_aux_pools[JEL_MANAGERS];
u32 _gz_jel_aux_private_apart(u32 pointer,u32 bytes);
void _gz_jel_manager_complete(u32 manager,u32 emitter_capacity,u32 aux_end,
                             u32 iterations,u32 configured);
#endif
