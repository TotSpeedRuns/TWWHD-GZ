#ifndef GZ_COLLIDER_OBSERVATION_H
#define GZ_COLLIDER_OBSERVATION_H
typedef unsigned int u32;
enum {CO_MAGIC=0x434f4231u,CO_CAP=8192,CO_EMPTY=0,CO_LIVE=1,CO_DEAD=2,CO_UNOBSERVED=3};
enum {CO_OVERFLOW=1,CO_EXHAUSTED=2,CO_REENTRY=4,CO_BAD_ARGUMENT=8,CO_MISSING_CTOR=16,
 CO_BAD_HEADER=32,CO_COUNTER_EXHAUSTED=64};
enum {CO_REMOVED=1,CO_PLACEMENT_REINIT=2,CO_SET_WITHOUT_CTOR=4};
typedef struct {
 u32 object,generation,state,born_frame,dead_frame,ctor_site,dtor_site,last_set_frame;
 u32 server,status,object_vtable,roles,set_count,remove_count,last_remove_frame,flags;
} CoEntry;
typedef struct {
 u32 magic,bytes,version,capacity,rows,busy,flags,serial,active;
 u32 ctor_count,dtor_count,set_count,remove_count,unknown_ctor_count,dropped_count,session;
} CoHeader;
typedef struct {u32 valid,flags,serial,session;CoEntry entry;} CoObservation;
typedef char CoEntrySize[(sizeof(CoEntry)==64)?1:-1];
typedef char CoHeaderSize[(sizeof(CoHeader)==64)?1:-1];
extern CoHeader _gz_co_header;
extern CoEntry _gz_co_rows[CO_CAP];
extern CoObservation _gz_co_observation;
/* Shared native hooks supply arguments at the exact original call boundary.
 * Constructor return and destructor entry never dereference the object.
 * Set reads only the original live cCcD_Obj header, never status or actors.
 * Remove never dereferences its object argument. No game memory is written.
 * Serial is an OBSERVATION stamp within this process, not native allocation,
 * original-factory, full lifecycle or cross-session authority. Session0 stays
 * explicitly unbound. No complete-coverage or replay-admission API is offered.
 */
void _gz_co_ctor_return(u32 object,u32 caller);
void _gz_co_dtor_enter(u32 object,u32 flags,u32 caller);
void _gz_co_set_enter(u32 server,u32 object,u32 caller);
void _gz_co_remove_enter(u32 server,u32 object,u32 caller);
/* Read-only native object query; only private observation output is written.
 * Finding a row, even LIVE, never grants a native pointer/lifetime certificate. */
u32 _gz_co_lookup(u32 object);
#endif
