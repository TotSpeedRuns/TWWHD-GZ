# Saved bomb identity

This authored component corrects a preparation mismatch after a saved bomb has
expired. The immutable constructor recipe intentionally has no issued process
ID yet. Its PID therefore cannot serve as the original saved bomb's identity.
The sealed private key now carries that authenticated saved PID separately.
The full immutable recipe, source validation and pair identity checks remain.
The public descriptor stays 64 bytes; private key storage grows by four bytes.

`bundle_coordinator.inc` is the complete authored unit against its selected
predecessor. The patch isolates the change and portable pins identify source and
compiled output. Whole predecessor reproduction, actual emitted code review and
the combined main's layout and packed-code checks passed. Native testing got
past the identity rejection but then reached the existing source-smoke coverage
guard. This does not fix full bomb restoration and is not a new released build.

Required native interfaces and the surrounding capture/restore pipeline are
external dependencies. This is a partial maintained component, not a standalone
builder. The complete integrated source export remains v126. No game assets,
memory captures, console profiles or personal codes are included.
