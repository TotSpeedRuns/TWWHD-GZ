# Native heap snapshot reference

Chris supplied [glankk/gz state.c](https://github.com/glankk/gz/blob/master/src/gz/state.c)
as a design reference. Its game-arena snapshot and allocator reconstruction are
relevant to reducing per-actor reconstruction. This is a study reference, not a
claim that its code has been ported or its addresses apply to Wind Waker HD.

For GZ, the next experiment is a bounded snapshot of one game-owned allocation
domain at a verified engine boundary. Before any restoration, determine which
allocations contain OS synchronization, graphics/audio ownership, resource
manager references or pointers crossing that domain. Capture storage and the
restore coordinator must remain outside the restored domain.

Acceptance requires repeated allocation/reuse tests, matching heap structure,
resource validity, and unchanged excluded domains. Broader coverage follows only
after those checks. Existing working builds and user checkpoints stay preserved.

A console-wide RAM overwrite is not implemented by this study. The current
experimental save-state integration remains separate from the v126 runtime.
