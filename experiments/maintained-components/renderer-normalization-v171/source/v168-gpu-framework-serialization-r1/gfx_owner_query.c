/* Unwired fixed native provider query. Zero mutable static storage.
 *
 * REQUIRED causal entry premise: genuine native Game framework frame from
 * 0274cb20 has entered its original graphics callback and has not reached
 * 0274cbf8. The sole-published native Graphics/ThreadMgr objects remain owned
 * by that entered native frame. This query does not turn a numeric address or
 * sampled mutex into arbitrary readable memory or a new graphics lease.
 * It accepts no caller-selected native pointer, never acquires/releases a
 * mutex, and cannot certify GPU producers which bypass the native lock.
 *
 * Use immediately before/after pre-HG DrawDone and again after HG acquisition.
 * Actual GHS/JWA identity is rechecked; a previous TRUE result is not a token.
 */
typedef unsigned int U;
typedef unsigned char B;
extern volatile U _gz_gfxq_graphics_singleton,_gz_gfxq_threadmgr_singleton;
extern const B _gz_gfxq_graphics_vtable[];
extern U _gz_ghs_context_owned(U),_gz_ghs_role(U,U),_gz_jwa_ghs_check(U);
extern U OSGetCurrentThread(void),OSGetThreadSpecific(U);
static U context(U req,U frame){U role;
 if(!req||!frame||!_gz_ghs_context_owned(req))return 0;
 role=_gz_ghs_role(req,frame);
 return (role==1u||role==2u)&&!_gz_jwa_ghs_check(req);
}
/* These are rejection checks only. The causal native-provider premise above
 * supplies lifetime; this numeric test supplies no memory-access authority. */
static U sane(U p,U n){return !(p&3u)&&p>=0x10000000u&&p<=0xbffe0000u-n;}
static void snapshot(U p,U*a){const volatile U*q=(const volatile U*)p;
 a[0]=q[3];a[1]=q[4];a[2]=q[5];a[3]=q[10];a[4]=q[17];a[5]=q[18];a[6]=q[21];}
U _gz_gfxq_owned(U req,U frame){U p,m,key,thread,tls,i,a[7],b[7];
 if(!context(req,frame))return 0;
 p=_gz_gfxq_graphics_singleton;m=_gz_gfxq_threadmgr_singleton;
 if(!sane(p,0x58)||!sane(m,0x28))return 0;
 key=*(volatile const U*)(m+0x24);
 thread=OSGetCurrentThread();tls=OSGetThreadSpecific(key);
 if(!thread||!tls)return 0;
 snapshot(p,a);
 if(a[0]!=(U)_gz_gfxq_graphics_vtable||a[1]!=tls||!a[2]||(a[2]&0x80000000u)||
  a[3]!=0x6d557458u||a[4]!=thread||!a[5]||(a[5]&0x80000000u)||a[6])return 0;
 __sync_synchronize();
 if(!context(req,frame)||OSGetCurrentThread()!=thread||OSGetThreadSpecific(key)!=tls||
  _gz_gfxq_graphics_singleton!=p||_gz_gfxq_threadmgr_singleton!=m||*(volatile const U*)(m+0x24)!=key)return 0;
 snapshot(p,b);
 for(i=0;i<7;i++)if(a[i]!=b[i])return 0;
 return context(req,frame);
}
