# Current job-owner allocation labels

This authored C experiment observes allocation metadata for two manager labels
from a genuinely completed native frame. It copies no manager, queue, vector,
worker or actor payload. A matching allocation generation is an identity;
it is not proof that the object cannot be destroyed or reused.

The private R844 build was exercised in one isolated Cemu run. Two fresh
observations, 300 native frames apart, produced four matching before/after
64-byte binding rows and sixteen genuine capture/validation returns of zero.
The general manager occupied a 128-byte allocation. The 112-byte render queue
was embedded inside a 912-byte allocation. These sizes describe that run;
they are not a license to copy either native object. All six save hashes were
unchanged. No hardware, physical lifetime, replay or world closure is claimed.

`owner_labels.c` is the reviewed source with line endings normalized to LF.
It uses the existing [object binding interface](../save_state/object_binding.h).
For a syntax-only check, supply that folder as an include directory. Linking
requires the authenticated frame/completion reports, existing OB metadata APIs,
native root aliases and the no-hold caller guard. The normal OB APIs can spin
on their private metadata lock; do not call this observer while a capture or
coupled job/heap hold is retained. Callbacks or supplied native results cannot
stand in for the actual producer and APIs.

The observer runs at most once per 300 frames. WORLD_READY and READ_READY stay
zero, with physical MISSING=31 and session zero. The later coupled exclusion
experiment is separate. This partial source module does not make a complete
new integrated release: the reproducible integrated export remains v126.
