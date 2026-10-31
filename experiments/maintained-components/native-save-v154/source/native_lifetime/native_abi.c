/* Cafe OSThread stackStart/+394, stackEnd/+398 from the bundled WUT ABI.
 * Real game stacks can be in MEM1 below10000000. Heap/object storage keeps
 * its separate strict DATA classification; only caller metadata uses this.
 * No pointer itself grants engine ownership or replay permission. */
#include "native_abi.h"
typedef unsigned int U32;
extern void *OSGetCurrentThread(void);
#define R(p,o) (*(volatile const U32*)((p)+(o)))
/* Strong fixed image owner guard linked from owned_images.o. */
unsigned int _gz_abi_metadata_span(U32 p,U32 n){U32 thread,top,bottom,stack,caller;
 if((p&3)||!n||p<0x01000000u||n>0xffffffffu-p)return 0;
 if(p>=0x10000000u&&p<0xbffe0000u&&n<=0xbffe0000u-p)return 1;
 thread=(U32)OSGetCurrentThread();
 if((thread&3)||thread<0x01000000u||thread>0xbffe0000u-0x39cu)return 0;
 top=R(thread,0x394);bottom=R(thread,0x398);
 __asm__ volatile("mr %0,1":"=r"(stack));
 if((top&3)||(bottom&3)||!bottom||top<=bottom||stack<bottom||stack>=top)return 0;
 /* Exclude this helper's own frame. A metadata pointer may refer to its
  * caller or an older active frame, not unused space below the live stack. */
 caller=R(stack,0);
 /* The owned native routines and emitted PPC use 8-byte stack frames;
  * 16-byte alignment at one outer frame is not an ABI-wide guarantee. */
 if((caller&7)||caller<stack||caller>=top)return 0;
 return p>=caller&&p<top&&n<=top-p;
}
