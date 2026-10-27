# Common callback-admission primitive

This standalone C component coordinates shared callback lifetimes and a
nonwaiting exclusive attempt. It is groundwork for retained-memory save states.
It does not capture, restore or grant access to game memory.

`common_admission.c` and `common_admission.h` match the reviewed R2 implementation
apart from canonical LF line endings. The caller owns one cold, zero-initialized
864-byte Gate.
Its 24-byte shared and 16-byte exclusive tokens bind the exact Gate identity.
Wrong-Gate cleanup and attempts to reuse active tokens preserve the original
cleanup token. Slots, nesting and monotonic generations are checked explicitly.

The metadata mutex is released before callbacks run. Closed shared entrants
use a condition variable; the exclusive TRY never waits for an active callback.
The owner cannot reenter shared work through the same Gate. Initialization is
once per live Gate, and OS mutexes, conditions, tokens and generations must not
be replayed as snapshot bytes. Breaking admission does not terminate readers
or establish safe object retirement.

The exact implementation passed a private Cemu test with four real OS threads
running on guest cores 0, 1 and 2, nested and overlapping shared calls, a blocked
entrant, busy and idle exclusive attempts, two-Gate token collisions and active
token retries. All four threads terminated and joined. This is a scoped runtime
observation, not a proof for every schedule or native Wii U behavior. The tests
did not establish game-worker, allocator, resource or object-lifetime closure.

Compile the object with devkitPPC and official WUT headers, for example:

```sh
powerpc-eabi-gcc -mcpu=750 -meabi -mhard-float -msdata=none -std=c11 \
  -O2 -Wall -Wextra -Werror -ffreestanding -fno-builtin \
  -fno-merge-constants -fno-common -fno-stack-protector \
  -fno-tree-loop-distribute-patterns -fno-unwind-tables \
  -fno-asynchronous-unwind-tables -fstack-usage \
  -I"$DEVKITPRO/wut/include" -c common_admission.c -o common_admission.o
```

The object requires genuine Cafe OS mutex, condition and current-thread APIs.
Compiling it does not supply those bindings or turn it into a complete program.
The private native test resolved the required real exports; no positive API
return values were supplied by a fixture. Source hashes and the tested object
hash are recorded in `scope.json`.

The reproducible integrated public export remains v126. This directory does
not reproduce a later complete release, add a gameplay rewind implementation,
or establish cross-room, camera, held-item, cutscene or persistent-slot coverage.
