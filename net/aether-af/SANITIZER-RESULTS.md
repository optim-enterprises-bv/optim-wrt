# af_match sanitizer and stack results

The parsing in `af_match.c` runs in ring 0 on attacker-controlled bytes, so
"the tests pass" is not sufficient evidence. This records what was actually
run, by whom, and — more importantly — what it does not cover.

## Runs

| | Result |
|---|---|
| `make check` (unsanitised) | 185 checks, 0 failures |
| `make asan` (ASan + UBSan, -O1 -g) | **185 checks, 0 failures** |
| `make fuzz` (randomised, sanitised) | **200,000 iterations, no trap**, `detect_leaks` clean |

The sanitised runs were performed by the firmware lane on a host with the
runtime, after **verifying the sanitizer actually traps a known
heap-buffer-overflow** — a clean result from a non-functioning sanitizer is
worse than no result, because it reads as evidence.

## Why the fuzz harness is worth more than its iteration count

Two properties, neither of which is volume:

**Every buffer is `malloc`'d to exactly the length passed in.** If the parser
reads one byte past `len`, a static array or an over-allocated buffer absorbs
it silently and the sanitizer sees nothing. An exact-sized heap allocation
turns the same read into a `heap-buffer-overflow` with a stack trace. The
fixed vectors in `test_match.c` mostly use static arrays and structurally
cannot cover that class.

**Inputs are seeded, then mutated.** A valid TLS ClientHello and an HTTP
request line are truncated at a random offset and mutated, with mutations
biased toward the first 16 bytes where the length fields live. Pure random
noise bails at byte 0 and tests nothing; corrupting a *valid* record gets the
parser deep enough to reach the code that matters.

`out_len` is also varied down to 1 byte on a quarter of iterations, exercising
the refuse-rather-than-truncate contract under pressure.

## Kernel stack, measured on the target architecture

`-fstack-usage`, aarch64, `-O2`:

| Function | Frame | Kind |
|---|---|---|
| `af_extract_sni` | **32 bytes** | static |
| `af_extract_http_host` | 16 bytes | static |
| `af_hashset_*`, `af_hash_name`, `af_name_plausible` | 0–32 bytes | static |

Every frame is **static** — no VLAs, no `alloca`, no recursion. The largest
on-stack object in the hook path is `char name[254]` in `af_hook`.

Worst case for the whole path is roughly **400 bytes** (254 name + 32 parser
frame + header structs + hook overhead) against arm64's 16 KB kernel stack,
about 2.5%. Kernel stack exhaustion is a real ring-0 failure mode that a host
build cannot surface, which is why it is measured rather than assumed.

## What none of this proves

Sized honestly, because the numbers above are easy to over-read:

- **Only `af_extract_sni` and `af_extract_http_host` were fuzzed.**
  `af_hashset_*` and all module glue were not.
- **200,000 iterations of one mutation strategy.** A structure-aware fuzzer
  would reach states this one does not. This is not a substitute for one.
- **Host build, x86-64.** Overreads transfer to the target — a read in bounds
  on the host is in bounds anywhere — but the kernel allocator, kernel stack
  limits and target codegen do not appear at all. The stack measurement above
  is the separate answer to that.
- **The module has never been loaded.** Compiling proves syntax; it proves
  nothing about behaviour under live traffic.
