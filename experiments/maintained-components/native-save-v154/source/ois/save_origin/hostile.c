/* TEST ONLY compiled hostile integer ABI callbacks; no origin authority. */
typedef unsigned int U;
__attribute__((visibility("hidden"))) U _gz_spo_private[32]={0x53504f31u,1,128};
static volatile U seen[4];
__attribute__((noinline)) static U hostile(U a,U b,U c){seen[0]=a;seen[1]=b;seen[2]=c;
 __asm__ volatile("li 0,37\nmtxer 0\nmtctr 0\nmtcrf 255,0\nli 3,3\nli 4,4\nli 5,5\nli 6,6\nli 7,7\nli 8,8\nli 9,9\nli 10,10\nli 11,11\nli 12,12\n":::"r0","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12","ctr","xer","cc","memory");return 28;}
U _gz_spo_outer_note(U a){return hostile(a,0,0);}
U _gz_spo_pre_note(U a,U b){return hostile(a,b,0);}
U _gz_spo_factory_note(U a,U b,U c,U d,U e){return hostile(a,b,c)^d^e;}
U _gz_spo_return_note(U a,U b,U c){return hostile(a,b,c);}
U _gz_spo_ctor_note(U a,U b,U c,U d){return hostile(a,b,c)^d;}
U _gz_spo_initpre_note(U a,U b,U c){return hostile(a,b,c);}
U _gz_spo_initpost_note(U a,U b,U c){return hostile(a,b,c);}
U _gz_spo_dtor_note(U a){return hostile(a,0,0);}
U _gz_spo_reentry_note(void){return hostile(0,0,0);}
