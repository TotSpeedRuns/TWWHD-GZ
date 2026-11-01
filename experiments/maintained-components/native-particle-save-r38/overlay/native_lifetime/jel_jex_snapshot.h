#ifndef GZ_JEL_JEX_SNAPSHOT_H
#define GZ_JEL_JEX_SNAPSHOT_H
#include <stddef.h>

/* Typed observer metadata, never native object or replay authority. Existing
 * JEL/JEX layouts remain unchanged. This header follows their JEL definitions. */
enum { JEX_CAP=512, JEX_DEPTH=4, JEX_OVERFLOW=1, JEX_LOST=2,
       JEX_CONTEXT=4, JEX_WRAP=8 };
typedef struct {u32 serial,kind,frame,thread,manager,manager_epoch,emitter,generation;
 u32 resource,group,index,scope,actor,pid,actor_generation,entry_procedure;} JexEvent;
typedef struct {u32 manager,emitter,generation,scope,actor,pid,actor_generation,
 entry_procedure,thread,frame;} JexOwner;
typedef struct {u32 cookie,actor,pid,generation,procedure,frame,thread,request;} JexScope;

enum { JEX_CAPTURE_MAGIC=0x4a585331u, JEX_CAPTURE_VERSION=1,
 JEX_CAPTURE_HEADER=64, JEX_CAPTURE_BYTES=10600,
 JEX_CAPTURE_REPORT=64, JEX_CAPTURE_OWNERS=160, JEX_CAPTURE_SCOPES=10400,
 JEX_CAPTURE_DEPTH=10528, JEX_CAPTURE_COOKIE=10532, JEX_CAPTURE_RETURNED=10536,
 JEL_JEX_SNAPSHOT_BYTES=21992 };
typedef struct {
 u32 magic,bytes,version,valid,request,frame,thread,po_frame;
 u32 jel_serial,jel_events,owner_rows,owner_bytes,scope_rows,scope_bytes,returned_words,reserved;
 u32 report[24]; JexOwner owner[JEL_ROWS]; JexScope scope[JEX_DEPTH];
 u32 depth,cookie,returned[JEX_CAP/32];
} JexCaptureSnapshot;
typedef struct {
 JelState ledger; JelPool pools[JEL_MANAGERS]; JelAuxPool aux_pools[JEL_MANAGERS];
 JexCaptureSnapshot jex;
} JelJexSnapshot;
typedef char JexEventSize[sizeof(JexEvent)==64?1:-1];
typedef char JexOwnerSize[sizeof(JexOwner)==40?1:-1];
typedef char JexScopeSize[sizeof(JexScope)==32?1:-1];
typedef char JexCaptureSize[sizeof(JexCaptureSnapshot)==JEX_CAPTURE_BYTES?1:-1];
typedef char JelJexCaptureSize[sizeof(JelJexSnapshot)==JEL_JEX_SNAPSHOT_BYTES?1:-1];
typedef char JexReportOffset[offsetof(JexCaptureSnapshot,report)==JEX_CAPTURE_REPORT?1:-1];
typedef char JexOwnerOffset[offsetof(JexCaptureSnapshot,owner)==JEX_CAPTURE_OWNERS?1:-1];
typedef char JexScopeOffset[offsetof(JexCaptureSnapshot,scope)==JEX_CAPTURE_SCOPES?1:-1];
typedef char JexDepthOffset[offsetof(JexCaptureSnapshot,depth)==JEX_CAPTURE_DEPTH?1:-1];
typedef char JexCookieOffset[offsetof(JexCaptureSnapshot,cookie)==JEX_CAPTURE_COOKIE?1:-1];
typedef char JexReturnedOffset[offsetof(JexCaptureSnapshot,returned)==JEX_CAPTURE_RETURNED?1:-1];

/* The real native lifetime TU admits only its one exact fixed receiver.
 * mode1 is one initial export; mode2 is read-only exact recheck. Authentication
 * runs before/after the JEL lock, never through an injected function pointer. */
u32 _gz_jpc_export_receiver(u32,u32,u32,const JelJexSnapshot*,u32,u32);
u32 _gz_jel_jpc_export(u32,u32,u32,JelJexSnapshot*,u32);
u32 _gz_jel_jpc_recheck(u32,u32,u32,const JelJexSnapshot*,u32);
#endif
