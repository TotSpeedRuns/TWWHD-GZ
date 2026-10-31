/* Both entries retain one event lock and one manager completion serial.
 * Only the genuine native return wrapper supplies the auxiliary register flow.
 * A layout receipt is not an allocation-generation or replay certificate. */
static void manager_registered(u32 p,u32 capacity,u32 complete,u32 end,u32 iterations,u32 configured){
 JelManager*m;JelPool*q;JelAuxPool*a;u32 i,n,bytes=0,base=0;
 _gz_jel_native_audit[0]++;_gz_jel_native_audit[1]=p;_gz_jel_native_audit[2]=capacity;
 /* Failed native self-allocation reaches this same epilogue. Its other
  * registers are unrelated inputs and must not become a pool receipt. */
 if(!p)return;
 if((p&3)||capacity>JEL_ROWS){poison(JEL_BAD);return;}
 q=pool(p);a=q?auxiliary_pool(q):0;
 if(complete){
  /* Construction entry invalidates both receipts before any native store.
   * The emitter array must have returned under that same construction, but
   * neither receipt may already carry a manager completion epoch. */
  if(!q||q->phase!=2||!q->construction||q->serial<=q->construction||
   q->capacity!=capacity||q->epoch||q->reserved||
   !a||a->manager!=p||a->construction!=q->construction||
   a->phase!=JEL_AUX_CONSTRUCTING||a->base||a->count||a->serial||a->epoch||a->reserved){
   poison(JEL_UNSEEN);return;
  }
  /* At the existing constructor epilogue r30 is one-past the auxiliary
   * array and r28 is its completed loop count. The wrapper also supplies
   * the actual configured count from current manager+0x2c. This metadata
   * observer reads no list head or native pool byte. */
  if(iterations!=configured||(end&3)||
   configured>(0xffffffffu-JEL_AUX_COOKIE)/JEL_AUX_STRIDE){poison(JEL_BAD);return;}
  bytes=configured*JEL_AUX_STRIDE;
  if(end<bytes){poison(JEL_BAD);return;}
  base=end-bytes;
  if((base&3)||(!base&&configured)||(base&&base<JEL_AUX_COOKIE)){poison(JEL_BAD);return;}
  /* count0 deliberately preserves base=end. Null/null denotes an absent
   * zero-element array only; a non-null zero array still has an8B cookie
   * whose native product word is1. A current OB/cookie join remains
   * mandatory before any native array payload read. */
 }
 m=manager(p);
 if(complete&&m&&m->epoch){poison(JEL_UNSEEN);return;}
 if(!m){for(i=0;i<JEL_MANAGERS;i++)if(!_gz_jel_state.manager[i].pointer){m=_gz_jel_state.manager+i;break;}}
 if(!m){poison(JEL_LIMIT);return;}
 n=next();if(!n)return;
 for(i=0;i<JEL_ROWS;i++)if(_gz_jel_state.row[i].manager==p){if(_gz_jel_state.row[i].alive)jex_ending(_gz_jel_state.row+i,3);_gz_jel_state.row[i].alive=0;}
 m->pointer=p;m->epoch=n;m->capacity=capacity;
 if(q&&q->phase==2&&q->capacity==capacity&&q->construction&&q->serial)q->epoch=n;
 if(complete){
  a->base=base;a->count=configured;a->serial=n;a->epoch=n;
  a->phase=JEL_AUX_COMPLETE;
 }else if(a){
  /* Legacy registration keeps its manager behavior. It cannot manufacture
   * auxiliary proof or refresh a previous auxiliary completion generation. */
  a->base=a->count=a->serial=a->epoch=0;a->phase=JEL_AUX_UNPROVED;
 }
}
void _gz_jel_manager(u32 p,u32 capacity){
 if(!enter(1))return;
 manager_registered(p,capacity,0,0,0,0);
 leave();
}
void _gz_jel_manager_complete(u32 p,u32 capacity,u32 end,u32 iterations,u32 configured){
 if(!enter(1))return;
 manager_registered(p,capacity,1,end,iterations,configured);
 leave();
}
