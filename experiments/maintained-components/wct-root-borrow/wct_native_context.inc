/* Fixed NBD→HBP lexical producer, not a setter of GFO/JOL readiness. */
extern u32 _gz_nbd_context_owned(u32);
typedef struct {u32 active,halted,serial,frame,thread;} WctNativeContext;
static WctNativeContext whl_native __attribute__((section(".bss.gz_whl_native")));
u32 _gz_wct_native_owned(void){u32 previous,restored;
 if(!whl_native.active||whl_native.halted||!whl_native.serial||
  !_gz_nbd_context_owned(whl_native.frame)||whl_native.frame!=_gz_pbac_frame||
  _gz_hg_platform_thread()!=whl_native.thread)return 0;
 previous=(u32)OSDisableInterrupts();restored=(u32)OSRestoreInterrupts((BOOL)previous);
 if(!previous||restored)return 0;
 __asm__ volatile("sync":::"memory");
 return whl_native.active&&!whl_native.halted&&_gz_nbd_context_owned(whl_native.frame)&&
  whl_native.frame==_gz_pbac_frame&&_gz_hg_platform_thread()==whl_native.thread?whl_native.serial:0;
}
void _gz_wct_native_enter(void){u32 f=_gz_pbac_frame,thread,previous,restored;
 if(whl_native.active||whl_native.halted||!_gz_nbd_context_owned(f))return;
 /* Ordinary frames add no platform-query work. Only the actual compiled
  * chooser intent consumed by the later WCT step needs this scope. */
 if(!_gz_wct_request||_gz_wct_request==_gz_wct_ack||!_gz_wcm_requested(_gz_wct_request))return;
 if(whl_native.serial==BAD){whl_native.halted=1;return;}
 thread=_gz_hg_platform_thread();previous=(u32)OSDisableInterrupts();restored=(u32)OSRestoreInterrupts((BOOL)previous);
 if(!thread||!previous||restored)return;
 whl_native.frame=f;whl_native.thread=thread;++whl_native.serial;
 __asm__ volatile("sync":::"memory");whl_native.active=1;
}
void _gz_wct_native_exit(void){
 if(!whl_native.active)return;
 /* Preserve terminal evidence/tokens, but expire producer authority as the
  * actual callback leaves. Retention never manufactures an extended frame. */
 if(_gz_wct_heap_pending()||_gz_wct_root_pending())whl_native.halted=1;
 __asm__ volatile("sync":::"memory");whl_native.active=0;
}
