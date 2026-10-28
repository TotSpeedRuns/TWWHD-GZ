# MSS input order and completed-label latch

The maintained four-file C/header snapshot uses observed controller order:

| Type | Required order |
| --- | --- |
| Slow | Plus, stick movement, Plus |
| Fast | Plus, Plus, stick movement |
| Insta | Plus, B, stick movement; B and stick may share the closing sample |

Pause duration and swimming speed do not choose the type. Ordinary B while no
pause attempt is open cannot start one. Opening Plus establishes the current
stick baseline; an already deflected stick is not invented as a later movement.
The existing vector deadband is unchanged. Simultaneous Plus+B, Plus+stick on
the closing sample, and stick-before-B do not establish a supported order.

The 50-word trainer ABI remains: word36 is the current collector result,
word37 is the last completed display, and word42 is the collector phase.
Opening the next pause clears only the current result. Unknown or ambiguous
current order stays zero, so the existing display consumer preserves word37.
The native visit report is ABI2 and supplies duration/visit diagnostics only;
it never publishes an input type. Its retained publication word22 remains zero.
Pause frames remain a separate display metric.

The exact runtime still needs its input wrapper to supply held/trigger/release
bits, calibrated axes and the actual frame identity. Its outer caller updates
previous-held word11 after each classifier call. A delayed trigger while a
button remains held is not another press; a genuinely observed release+trigger
can establish one. The wrapper, native UI observation hooks, actor/epoch reset,
enable/context checks and display consumer remain integration dependencies.
No copied runtime assembly, native instruction data or host reader is added.

## Scoped observations

One private Cemu run of the integrated candidate completed twelve sequences:
Slow, Fast, Insta with B+stick, and Insta with B then stick, each after requested
paused dwell of 0.1, 0.3 and 0.8 seconds. Native pause metrics ranged from 4 to
27 frames while the types followed order. Each new pause retained the preceding
completed label, including a final unfinished pause retaining Insta. Ordinary
compiled controller input was the only host write interface; six original and
disposable save-file hashes stayed unchanged. The earlier setup script failed
before these cases due to a missing helper import and was preserved separately.

These are controller and trainer-metadata observations, not pixel checks,
Wii U chronology/hardware acceptance, all historical pad subsamples, or new
save-state coverage. This source update does not label a new delivered release:
the delivered revision remains v147, and the complete reproducible integrated
source export remains v126. Full current integration is still not exported.

## Portable classifier tests

`mss_order_cases.c` executes the actual `mss_order.c` classifier with scalar
controller sequences. It varies dwell, checks latch/neutral drift/duplicate
views, and rejects unsupported order. The void visit notification is a counter
stub only; no successful API, native visit, Gate or gameplay result is supplied.
The native visit module and UI hooks require their separate target integration.

With a C11 compiler from this directory:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror mss_order.c mss_order_cases.c -o mss-order-test
./mss-order-test
```

The repository Python test runs this portable case file when a compatible host
C compiler is available. Generated executables belong outside the source tree.
