/* Borrow the actual operation's immutable source. This READ view never owns,
 * acquires, releases or transfers its allocation pin, and grants no actor API.
 * Included after the original source bridge/access implementations. */
typedef struct {
 u32 magic,bytes,version,kind,token,frame,serial,pin;
 u32 slot,source,used,identity,generation,revision,reserved0,reserved1;
} SomBombView;
static SomBombView _gz_som_bomb_view;
extern u32 _gz_roh_gl_context(u32,u32,u32);
static u32 som_bomb_readable(u32 token,u32 frame,u32 deep){const volatile u32*o=_gz_roh_owner;
 if(deep>1||bsr_blocked()||!token||_gz_som_token!=token||!frame||frame!=_gz_roh_live_frame||
  o[0]!=0x524f4831u||o[1]!=240||o[2]!=2||o[4]!=1||o[5]!=token||o[7]||!o[11]||
  (o[3]==1?(o[6]||o[12]!=1||o[10]!=frame):
   (o[3]!=2||o[6]!=1||(o[12]&&o[12]!=9)||(o[12]&&o[10]!=frame)))||!som_live(_gz_som_slot))return 0;
 /* This query runs the original common guards directly, without invoking the
  * lifetime scope/provider callbacks or granting their physical authority. */
 return _gz_roh_gl_context(token,frame,o[3]==1&&deep?1:2)==1;
}
u32 _gz_som_bomb_source(u32 token,u32 frame,u32 deep){SomBombView*s=&_gz_som_bomb_view;u32 p,n,pin,serial,z;
 s->kind=0;
 if(!som_bomb_readable(token,frame,deep))return 0;
 pin=_gz_som_pin;serial=_gz_roh_owner[13];p=_gz_osa_source(pin);n=_gz_osa_bytes(pin);
 if(!p||n<216)return 0;
 z=_gz_b294m_archive_validate(p,n,deep);
 if(z||!som_bomb_readable(token,frame,0)||pin!=_gz_som_pin||serial!=_gz_roh_owner[13]||
  p!=_gz_osa_source(pin)||n!=_gz_osa_bytes(pin))return 0;
 s->magic=0x53425631u;s->bytes=64;s->version=1;s->token=token;s->frame=frame;s->serial=serial;s->pin=pin;
 s->slot=_gz_som_slot;s->source=p;s->used=n;s->identity=_gz_osa_state.pin.identity;
 s->generation=_gz_osa_state.pin.generation;s->revision=_gz_roh_registry_revision;s->reserved0=s->reserved1=0;
 __asm__ volatile("sync":::"memory");s->kind=_gz_roh_owner[3]==1?2:3;return (u32)s;
}
