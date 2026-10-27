# Single-address atomic counter helper

`counter.c` is the exact reviewed C helper used to avoid an indexed-atomic
translation defect in Cemu 2.6's x64 recompiler. `counter.h` supplies a C/C++
declaration. This is GCC-compatible C using `__sync_fetch_and_add`; it requires
no WUT or game header. It does not change the already published
[common-admission Gate](../common-admission/README.md).

The PowerPC indexed `stwcx.` instruction must preserve its address registers.
At official Cemu commit `a6fb0a48eb437a8a41c13b782ac8ae0433bf8f98`, the
[x64 STWCX path](https://github.com/cemu-project/Cemu/blob/a6fb0a48eb437a8a41c13b782ac8ae0433bf8f98/src/Cafe/HW/Espresso/Recompiler/PPCRecompilerX64.cpp#L589-L646)
adds the index to the host base register and omits its restoration. The nearby
[LWARX path](https://github.com/cemu-project/Cemu/blob/a6fb0a48eb437a8a41c13b782ac8ae0433bf8f98/src/Cafe/HW/Espresso/Recompiler/PPCRecompilerX64.cpp#L456-L471)
restores that base. The official source blob for this file is
`97b2c14c5b6cc95803769f4503cc3218f1e10333`.

The compiler-generated PowerPC in the diagnosed case was correct. Reusing its
base after the affected indexed store explained the two observed adjacent
counter corruptions exactly. Effects depend on host-register allocation.
The later emulator crash was a separate issue; this helper is not a general
Cemu crash fix or a snapshot/lifetime-closure implementation.

Keep `counter.c` in a separate translation unit without LTO. The caller bounds
the report index and passes the complete address before the C call:

```c
#include "counter.h"

/* The application owns this already-declared array. */
extern volatile uint32_t report[64];

static void count(uint32_t index)
{
    if (index < 64)
        _gz_jwa_increment_counter(&report[index]);
}
```

Compile the helper as C, including when callers are C++:

```sh
powerpc-eabi-gcc -mcpu=750 -meabi -mhard-float -msdata=none -std=c11 \
  -O2 -Wall -Wextra -Werror -ffreestanding -fno-builtin -fno-lto \
  -c counter.c -o counter.o
powerpc-eabi-objdump -dr counter.o
```

Audit the emitted code after compiler/flag changes. The reviewed 28-byte leaf
uses `lwarx value,0,address` and `stwcx. value,0,address`, with RA=0 for both;
the complete address is in `r3`. It retains the builtin's barriers and only
uses volatile `r9`/CR0. It needs no frame or native API.

The reviewed adapter had 26 atomic instructions and every RA field was zero.
Its source/bounds/ABI review approved private component composition. That is
compiled-code evidence, not proof of all schedules, new OS API results, game
writer completeness or Wii U behavior. Source and official-reference identities
are recorded in `scope.json`. No game files, full adapter, compiled objects,
native results or diagnostic captures are included.

The shipped v145 release and reproducible integrated v126 export are unchanged.
Authored project code is MIT licensed under the repository's root license.
