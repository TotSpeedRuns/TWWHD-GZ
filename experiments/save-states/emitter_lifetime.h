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
