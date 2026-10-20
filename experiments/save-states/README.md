# Experimental save-state components

These are the current independent C/header/assembly components from the private
save-state development branch. They are not part of the v125 release payload.
They remain incomplete and must not be substituted into a console build without
their native integration, ownership, allocation and scheduler contracts.

The latest integrated private ELF passed 202 scoped compiled-code checks. These
checks cover selected native callback paths and explicit resource/heap boundary
models; they do not establish full-game save-state correctness. Native particle
writer exclusion, heap-generation binding, forward death revival, full actor
recreation and durable slots remain unfinished.

`emitter_lifetime.c` and `emitter_hooks.S` include the corrected constructor
capacity source and witnessed pool construction/stride/manager epochs.
