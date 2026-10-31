/* Whole owned allocator-domain diagnostic. The retained two-target GAC
 * implementation above stays unchanged; no KRC/JPG/GFO grant is reused. */
#include "owned_heap_native.h"
static struct {U phase,request,frame,thread,result;} gac_heap;
U _gz_gac_heap_private_apart(U p,U n){return n<=0xffffffffu-p&&
 (p+n<=(U)&gac_heap||(U)&gac_heap+sizeof(gac_heap)<=p);}
U _gz_gac_heap_scope(U req,U f,U phase){
 if(!gac_heap.phase||gac_heap.request!=req||gac_heap.frame!=f||
  !gac_heap.thread||gac_heap.thread!=_gz_hg_platform_thread())return 0;
 if(phase==1)return gac_heap.phase==1&&_gz_wct_ghc_scope(req,f,1);
 if(phase==2)return gac_heap.phase==2&&_gz_wct_ghc_scope(req,f,2);
 if(phase==3)return gac_heap.phase<=4&&_gz_wct_ghc_scope(req,f,3);
 return 0;
}
U _gz_gac_heap_reserve(U req,U f){U cap,z;
 if(gac_heap.phase||!_gz_wct_ghc_scope(req,f,1))return GHC_CONTEXT;
 gac_heap.phase=1;gac_heap.request=req;gac_heap.frame=f;
 gac_heap.thread=_gz_hg_platform_thread();gac_heap.result=0;
 cap=_gz_ghc_propose(req,f);if(!cap){gac_heap.result=GHC_OBSERVER;_gz_ghc_report[31]=GHC_OBSERVER;return GHC_OBSERVER;}
 z=_gz_ois_heap_reserve(req,f,cap);if(z){gac_heap.result=z;_gz_ghc_report[31]=z;return z;}
 z=_gz_ghc_refresh(req,f);gac_heap.result=z;_gz_ghc_report[31]=z;return z;
}
U _gz_gac_heap_read(U req,U f){U z;
 if(gac_heap.phase!=1||gac_heap.request!=req||gac_heap.frame!=f||
  gac_heap.thread!=_gz_hg_platform_thread()||!_gz_wct_ghc_scope(req,f,2))return GHC_CONTEXT;
 gac_heap.phase=2;z=_gz_ois_heap_read(req,f);
 gac_heap.result=z;gac_heap.phase=z?4:3;_gz_ghc_report[31]=z;return z;
}
U _gz_gac_heap_finish(U req,U f,U success){U z,release;
 if(!gac_heap.phase){return _gz_wct_ghc_scope(req,f,3)?0:GHC_RETAINED;}
 if(!_gz_gac_heap_scope(req,f,3))return GHC_RETAINED;
 release=_gz_ghc_release(req,f);if(release==GHC_RETAINED)return release;
 z=_gz_ois_heap_finish(req,f,success&&gac_heap.phase==3&&!gac_heap.result&&!release);
 if(z==GHC_RETAINED)return z;
 gac_heap.phase=0;_gz_ghc_report[31]=release?release:z;return release?release:z;
}
