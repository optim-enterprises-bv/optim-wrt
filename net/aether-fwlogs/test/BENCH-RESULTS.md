# ADR-020 gate: classification throughput on BPI-R4

Measured 2026-08-22. Reproduce with `bench-classify.sh` and `mkflows.py`.

## Device under test

| | |
|---|---|
| Board | Banana Pi BPI-R4 (2x SFP+), MediaTek filogic |
| Kernel | 6.18.44 |
| Cores | 4 |
| RAM | 4,022,660 kB (~3.8 GB) |
| Classifier | nDPId 1.7.0-release over libnDPI 5.0 |

## Result

Two pcaps with an identical packet population (100,000) split across different
flow counts, so per-flow cost separates from per-packet cost. Three runs each,
on an otherwise idle board.

| Flows | Packets | Wall time (3 runs) | flows/sec | packets/sec | Peak RSS |
|---|---|---|---|---|---|
| 20,000 | 100,000 | 5.19 / 5.20 / 5.19 s | **3,850** | 19,250 | 35.9 MB |
| 2,000 | 100,000 | 1.34 / 1.33 / 1.34 s | 1,500 | 75,000 | 29.8 MB |

Solving the pair:

```
20,000·F + 100,000·P = 5.19
 2,000·F + 100,000·P = 1.34
```

| | |
|---|---|
| **Per-flow cost** | **214 µs** |
| **Per-packet cost** | **9.1 µs** |

The model reproduces both measurements to the centisecond (5.19 s and 1.34 s
predicted). Flow-setup-only ceiling is ~4,670 flows/sec; with the five packets
each flow carries, the sustained figure is **~3,850 new flows/sec**.

**Cost is dominated by per-flow work, not per-packet work** — which is what
ADR-020 predicted, and why new-flow rate is the gate rather than throughput.
The per-packet path is fast: with long-lived flows the same board sustains
75,000 packets/sec.

## Verdict

**PASS on filogic.** A subscriber gateway sees tens to low hundreds of new
flows per second at peak; a busy household with many IoT devices, some
hundreds. 3,850/sec is roughly 10–40× that, on two of four cores.

**NOT a pass on IPQ5018.** The BPI-R4 is the optimistic case — 4 cores and
3.8 GB of RAM. The D50/IPQ5018 lane is materially weaker and has not been
measured. The gate is per-SoC and is only cleared for filogic.

Memory is worth noting separately: 35.9 MB resident for 20,000 concurrent
flows is nothing on 3.8 GB, but it is ~28% of a 128 MB device. The bounded
flow table required by ADR-020 decision 8 is not optional on small targets.

## What this measurement does not say

Read before quoting the headline number.

- **nDPId is a proxy, not `aether-sensord`.** It shares libnDPI 5.0 and an
  equivalent capture path, but it does not run our 1,347-signature matcher,
  our policy engine, or any enforcement. Our per-flow cost will be **higher**.
- **It also pays a cost we would not.** nDPId serialises a JSON record per flow
  to a separate collector process; that collector consumed 1.73 s of CPU per
  20,000 flows (~86 µs/flow) in a side measurement. `aether-sensord` matches
  in-process and emits aggregates, so some of the 214 µs is architecture we
  are not adopting. These two effects push in opposite directions and the net
  is unknown until our own daemon exists.
- **pcap replay from tmpfs excludes live-capture overhead** — AF_PACKET /
  TPACKETv3 ring handling, kernel copies, ring contention.
- **The flow population is synthetic.** It is a rate ceiling under an
  exactly-countable load, not field traffic, and no classification result from
  it is evidence that anything works in production (ADR-003). The realism leg
  of the gate still needs a capture of real traffic.
- **It says nothing about hardware-offload bypass.** That is a separate gate
  item and cannot be measured by replay — offload is bypassed precisely
  because packets never reach the classifier, which a file replay cannot
  reproduce.

## Method notes

- Benchmark instances write to a throwaway `nDPIsrvd` collector on a separate
  socket, so no synthetic flow reaches the live analytics pipeline.
- busybox `date` has no `%N`; `/proc/uptime` is the portable clock (centisecond
  resolution, present on every OpenWrt target).
- An earlier run set showed 7.25 → 9.86 s and was discarded: an abandoned
  `nDPId-testing-test` process from a timed-out invocation was still running at
  182 MB RSS and contending for CPU. Always check `load_before` in the results
  header before trusting a number.
- `tcpdump-mini` was installed on the board for packet counting and remains
  installed.
