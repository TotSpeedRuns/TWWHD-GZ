#ifndef GZ_PROCESS_OBSERVER_H
#define GZ_PROCESS_OBSERVER_H
typedef unsigned int u32;typedef unsigned short u16;typedef unsigned char u8;
enum {PO_MAGIC=0x504f4231u,PO_CAP=8192,PO_EMPTY=0,PO_LIVE=1,PO_ENDING=2,PO_ENDED=3};
enum {PO_BAD_HEADER=1,PO_REENTRY=2,PO_BAD_PROCESS=4,PO_OVERFLOW=8,
 PO_SERIAL_EXHAUSTED=16,PO_COUNTER_EXHAUSTED=32,PO_MISSING_BEGIN=64,
 PO_REPLACED_LIVE=128,PO_AMBIGUOUS_OWNER=256,PO_PROFILE_CHANGED=512};
enum {PO_ROW_REPLACED_LIVE=1};
typedef struct {
 u32 object,generation,state,base_bytes,other_bytes,allocation_bytes,pid,profile;
 u32 process_name,profile_entry,born_frame,end_frame,birth_caller,end_caller,flags,init_state;
} PoEntry;
typedef struct {
 u32 magic,bytes,version,capacity,rows,busy,flags,serial,active;
 u32 created,ending,ended,dropped,unmatched,ambiguous,session;
} PoHeader;
typedef struct {u32 valid,result,flags,session;PoEntry entry;} PoObservation;
typedef char PoEntrySize[(sizeof(PoEntry)==64)?1:-1];
typedef char PoHeaderSize[(sizeof(PoHeader)==64)?1:-1];
extern PoHeader _gz_po_header;
extern PoEntry _gz_po_rows[PO_CAP];
extern PoObservation _gz_po_observation;
extern volatile u32 _gz_po_frame;
extern const u32 _gz_po_profiles[484];
/* Observe the exact native base-process Create return. The native allocation
 * and profile/header initialization have completed; actor-specific constructor
 * readiness has not. Frame/request/PID/profile and exact base+other byte count
 * are observations, not a stable heap/thread or physical allocation certificate.
 * This function reads only that live base header and its native profile.
 */
void _gz_po_created(u32 process,u32 caller);
/* Exact successful Delete path, immediately before cMl::free and immediately
 * after it returns. Neither function reads process/profile/heap memory. Ending
 * excludes the row from owner queries before a potentially retiring call.
 * Ended records call completion only; no free-result scalar is interpreted as
 * physical reclamation, recursive release or complete reverse-reference proof.
 */
u32 _gz_po_end_begin(u32 process,u32 caller);
void _gz_po_end_return(u32 process,u32 boundary,u32 generation);
/* Private fixed output only. Query never dereferences the object/span or a
 * saved pointer. owner_of requires one unique observed LIVE enclosing process
 * allocation. Any recorded observation gap returns3 with no valid owner.
 * Finding it alone grants no native lifetime/replay authority.
 * Session remains zero/unbound and there is deliberately no admission API.
 */
u32 _gz_po_lookup(u32 process);
u32 _gz_po_owner_of(u32 pointer,u32 bytes);
#endif
