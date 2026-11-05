This unselected source snapshot contains the reviewed logical JOL after-frame → JWA helper → terminal-safe initializer edge. The helper lives in the coherent provider TU and uses its existing private state and Gate. JOL calls it after its existing GFO/current-thread/current-frame and ordinary-context checks, before its unchanged cohort and read-admission path.

Build with Python 3.9 or newer, devkitPPC r49 and genuine WUT 1.3.0 headers. GCC's `stdint.h`/`stddef.h`, WUT's coreinit headers and their genuine transitive headers are external dependencies. No SDK source is included. Pass the roots explicitly:

```text
python tools/build_jwa_after_frame_components.py --devkitppc /path/to/devkitPPC --wut /path/to/wut --output ../jwa-frame-build
```

The output must be new and outside the repository, with an existing parent. The builder derives its source root from its own location, compiles both exact authored TUs and performs a relocatable `-r` link. It reads no recovery folder, game, profile or prebuilt object. The reviewed outputs are:

| Output | SHA-256 |
| --- | --- |
| `frame-init-provider.o` | `2da93f1d55de327ae4f171381aacd8a92dc013cec76c7bf4ee4530d90a2d5905` |
| `frame-init-jol.o` | `ebc57ccfc879e5e4d9859d714c2c1f948a2243c7540b691395aaa349d93ba725` |
| `frame-init-components.o` | `92c2858ab198b568171b52293c7399d501ed0eccb397e1bb175b4da11cfab9fe` |

The source and build snapshot are manifest-pinned. The provider's 43 earlier methods and fixed 352-byte initializer are unchanged; the new helper adds 344 CODE bytes and JOL grows by 40. No mutable storage, owner or SDK import is added. Other owned helper sources are included as authored dependencies, without linking extra implementations or filling unresolved roles with stubs.

An entry observation of non-COLD state returns before SDK/GFO/frame queries. COLD preflight is not atomic with other state writers; the final check and real constructor CAS arbitrate initialization. The helper authenticates GFO serial/AFTER_RETURN phase/current thread and separately checks the current-frame role. It does not require or establish GFO entry-frame equality. Existing poison, cold-record/drain and WORLD/READ gates remain unchanged.

The link resolves local logical source edges only. External JOL invocation, Native activation, cold-before-use, same-owner physical mapping/lifetime and full consumer closure remain unproved. Building this snapshot does not produce a practice game, Setup app or complete Source712 module; those require the other owned integration inputs and real bindings. No hardware, B43, crash or lag improvement is established by this source snapshot.
