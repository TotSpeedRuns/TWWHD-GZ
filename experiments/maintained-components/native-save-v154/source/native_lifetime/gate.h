#ifndef GZ_NATIVE_CAPTURE_EXCLUSION_H
#define GZ_NATIVE_CAPTURE_EXCLUSION_H
typedef unsigned int u32;
enum {HG_MAGIC=0x48474331u,HG_TICKET=0x48475431u,HG_EXCLUSIVE=0x80000000u,HG_COUNT=0x7fffffffu};
enum {HG_OK=0,HG_BUSY=1,HG_OWNER=2,HG_CONTEXT=3,HG_INTEGRITY=4};
enum {HG_BAD_HEADER=1,HG_BAD_TICKET=2,HG_EXHAUSTED=4,HG_BAD_COUNT=8,HG_BAD_CONTEXT=16};
typedef struct {
 u32 magic,bytes,version,flags,state,serial,capture_serial,owner_lock;
 u32 token,owner_thread,ready,waiters,entered,exited,captures,result;
} HgControl;
typedef struct {u32 magic,self,serial,site,live,frame,reserved[2];} HgTicket;
typedef char HgControlSize[(sizeof(HgControl)==64)?1:-1];
typedef char HgTicketSize[(sizeof(HgTicket)==32)?1:-1];
extern HgControl _gz_hg_control;
extern volatile u32 _gz_hg_frame;
/* Every actual native mutator must be wrapped from entry through every
 * return before capture can use this component. A ticket is fresh, genuinely
 * mapped wrapper-owned STACK storage, never an archive/actor pointer. The
 * wrapper forwards the native function even if observation is refused.
 * Pass the exact enter result separately to exit; never reread a saved token
 * from a possibly replaced ticket. Missing coverage prevents native admission. */
u32 _gz_hg_native_enter(HgTicket*,u32 original_site);
u32 _gz_hg_native_exit(HgTicket*,u32 expected_serial);
/* One capture lane owns the operation lock until after release signals its
 * waiters. Begin is nonblocking if a mutation is active. This excludes known
 * wrapped heap mutators, NOT actor execution, GPU/audio or resource streams.
 * Actual native callback/context and complete entry coverage remain required. */
u32 _gz_hg_capture_begin(u32 context_cookie);
u32 _gz_hg_capture_seal(u32 token);
u32 _gz_hg_capture_end(u32 token);
/* Fixed linked platform/context providers, not user-supplied callbacks. */
u32 _gz_hg_context_owned(u32 cookie);
u32 _gz_hg_platform_thread(void);
void _gz_hg_event_init(void);
void _gz_hg_event_reset(void);
void _gz_hg_event_wait(void);
void _gz_hg_event_signal(void);
#endif
