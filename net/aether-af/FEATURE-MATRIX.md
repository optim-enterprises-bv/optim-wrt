# aether device stack — combined capability matrix

What the finished product does, where each capability comes from, and what
state it is in. Organised by capability rather than by source project, because
the point of the rewrite is that these stop being separate products.

Every licence, size and performance figure was verified during design.

**Status** — `SHIPPED` proven on the BPI-R4 · `BUILT` host-tested · `PLANNED`
designed and agreed · `REJECTED` deliberately not taken

**Source** — OAF · NFY (Netify) · SEN (Turris Sentinel) · PAK (Turris pakon) ·
nDPI · OSY (OpenSync) · AE (aether, ours)

---

## 1. Classification — knowing what the traffic is

| Capability | Source | Status |
|---|---|---|
| TLS SNI / HTTP Host extraction | OAF, nDPI | `PLANNED` |
|  **474** protocol dissectors | nDPI | `PLANNED` |
| **QUIC ClientHello decryption** | nDPI | `PLANNED` — retires `disable_quic` |
| **1,347-signature application database** | **AE** | `SHIPPED` — 15× upstream OAF's 87 |
| Label-prefix name matching | AE | `BUILT` — recovers `youtubei.googleapis.com`, rejects `evil-youtube.com` |
| **Stable tags, never numeric ids** | NFY | `SHIPPED` — kills the 11001-is-Samba class of bug |
| 4-axis taxonomy: proto / proto-cat / app / app-cat | NFY | `BUILT` |
| Signature-DB pinned by sha256 | AE | `PLANNED` — refuse to categorise on mismatch |
| Ambiguity detection (one host, many apps) | AE | `SHIPPED` — 27 contested patterns found in our own DB |
| nDPI per-flow risk flags | NFY via nDPI | `PLANNED` |
| Statistical / ML classification | — | `REJECTED` — z-scores are not DPI |

## 2. Enforcement — making it stop

| Capability | Source | Status |
|---|---|---|
| In-kernel per-MAC drop | OAF | `PLANNED` — `aether-af`, GPL mechanism |
| **First-packet blocking** | OAF | `PLANNED` — kills the handshake before the request is served |
| nftables reputation sets | NFY, SEN | `SHIPPED` — **a packet actually died** |
| TTL'd set elements | NFY | `SHIPPED` — decay enforced by the kernel |
| Enforcement survives daemon death | OAF | `PLANNED` — the actual justification for a kernel module |
| Blocked-page **redirect** | OSY `fsm_policy` | `PLANNED` — beats a silent drop for support load |
| TCP RST on block | OAF | `REJECTED` — the only GPL-only symbol. Silent drop instead |
| Abandon flows past 256 packets | OAF `by_pass_accl` | `REJECTED` — that is a bug |

## 3. Parental controls

| Capability | Source | Status |
|---|---|---|
| Per-MAC subjects | OAF | `BUILT` — a rule reaches exactly one subject |
| Time windows | OAF | `BUILT` — polarity stated, never inferred |
| Per-weekday daily quotas | OAF | `BUILT` — with the wrapping-window day fix |
| Category-level rules | NFY | `BUILT` |
| App exception inside a blocked category | AE | `BUILT` |
| Coverage gap surfaced at authoring time | AE / ADR-017 | `BUILT` — unknown tag refused, not silently dead |

## 4. Security & threat

| Capability | Source | Status |
|---|---|---|
| **Attacker reputation, fleet-sourced** | SEN | `SHIPPED` |
| delta / serial / gap-resync feed | SEN DynFW | `SHIPPED` — a gap leaves the set untouched |
| Firewall-drop sensing | SEN `fwlogs` | `BUILT` — 88 checks |
| Scoring: decay, breadth, hysteresis | AE | `BUILT` — `nemesis`, 57 checks |
| Absolute allowlist precedence | AE | `BUILT` |
| Spamhaus DROP ingest | AE | `BUILT` — the one licence-clean IP feed |
| UPnP / NAT-PMP abuse detection | OSY `sec_portmap` | `PLANNED` |
| Local TTL'd verdict cache | OSY `gatekeeper_cache` | `PLANNED` |
| **Honeypots / minipots** | SEN | `REJECTED` — bait services on subscriber CPE |
| HaaS cloud honeypot | SEN | `REJECTED` |

## 5. OpenSync Tier 1 — the high-value gaps

| Capability | Source | Status |
|---|---|---|
| **QoS / traffic shaping** | OSY `qosm` | `PLANNED` — highest value item. aether can classify and only block |
| Per-app / per-subject bandwidth ceilings | OSY | `PLANNED` |
| **Firewall rule management** | OSY `nfm` | `PLANNED` — port forward, DMZ, custom rules |
| **Cellular / modem management** | OSY `cellm` | `PLANNED` — separate crash domain |
| **WAN orchestration + failover** | OSY `wano`, `cm2` | `PLANNED` — separate crash domain |

## 6. OpenSync Tier 2 — worth doing

| Capability | Source | Status |
|---|---|---|
| VPN management (WireGuard) | OSY `vpnm` | `PLANNED` — separate crash domain |
| Captive portal / guest onboarding | OSY `cpm` | `PLANNED` |
| Scheduled speedtests | OSY `tpsm` | `PLANNED` — management plane |
| Diagnostic bundles | OSY `dm` | `PLANNED` — management plane |
| Service watchdog | OSY `wpd` | `PLANNED` — management plane |

## 7. OpenSync Tier 3 — situational

| Capability | Source | Status |
|---|---|---|
| Thread / Matter border router | OSY `iotm` | `REJECTED from sensord` — different radios, own package |
| BLE onboarding | OSY `blem` | `REJECTED from sensord` |
| Persistent config storage | OSY `psm` | `REJECTED` — platform service, `configs`/`delta` cover it |

## 8. Telemetry, analytics, privacy

| Capability | Source | Status |
|---|---|---|
| Flow / app telemetry to controller | nDPI, NFY | `PLANNED` |
| **On-device flow history, queryable locally** | PAK | `PLANNED` — privacy-positive, stays in the household |
| Health as composition, never a score | AE | `BUILT` |
| Bounded tables, counted refusals throughout | AE | `SHIPPED` |
| Per-component consent, off by default | SEN EULA model | `BUILT` |

## 9. Do we lose anything by rejecting walleye and netifyd?

No. Those rejections are about **code and data**, not capability. Verified
against nDPI 5.0 headers on this machine:

| | nDPI 5.0 |
|---|---|
| Protocol IDs | **474** |
| TLS | `server_names` (SNI), `advertised_alpns`, `negotiated_alpn`, `issuerDN`, `subjectDN` |
| QUIC | `NDPI_PROTOCOL_QUIC`, `quic_*` |
| Fingerprinting | `ja3_server`, `ja4_client` |
| Risk flags | 27 |

**walleye** is an `rts` bytecode VM plus a signature bundle. The VM is
BSD-3-Clause and portable; only the bundle is unlicensable. What the bundle
does — teach the VM to dissect protocols and emit `tls.sni` / `quic.sni` /
`http.host` / `dns.*` — is exactly what nDPI already does, with a licence,
more protocols, and native QUIC that walleye needs its bundle for. There is
nothing to duplicate. Its one unique asset is Plume's `service.application`
classifications; ours is the 1,347-signature database.

**Netify**, capability by capability:

| Netify | Ours |
|---|---|
| nDPI classification | same library, directly |
| Encrypted-traffic metadata | nDPI TLS fields |
| JA3/JA4 | nDPI |
| Flow risks | nDPI |
| 4-axis taxonomy, stable tags | `BUILT` |
| Device identification *(paid)* | `argus`, ADR-009 |
| **IP Sets / Flow Actions plugin *(paid)*** | **`SHIPPED`** |
| Stats aggregator *(paid)* | `prometheus-engine` |
| Message queue *(paid)* | USP / MQTT |
| Network intelligence ML *(paid)* | `nemesis` |
| **Signature updates *(paid)*** | **our own database** |

Every capability, including the paid ones, is built or reachable from the same
library. The paid parts are precisely the ones we replace with our own --
renting their feed would reintroduce the vendor-decay risk ADR-017 names.

What we genuinely lack is **assets, not architecture**: their curated
signature breadth, and their cloud ML. Both are bought or grown over time.

## 10. Transport & platform reach

| Capability | Source | Status |
|---|---|---|
| USP over WebSocket / MQTT / CoAP / STOMP / WRP | AE | exists — sensord owns **no** transport |
| Carried by `ac-client`'s existing mTLS identity | AE ADR-018 | `SHIPPED` |
| OpenWrt / prplOS | — | primary target |
| QSDK | — | `PLANNED` — kernel 3.18/4.4/5.10 lane |
| OpenSync | OSY `fsm_dpi_sni` | `PLANNED` — plugin, not a port. Same DB, different host |
| RDK-B | — | **deferred** — no firewall seam we control |

---

## Licence boundary

| Component | Licence | Contains |
|---|---|---|
| `aether-af` kernel module | **GPL-2.0** | mechanism: extract, hash, look up, drop |
| `aether-sensord` | **BSD-3-Clause** | signatures, matching, policy, taxonomy, feed |
| `aether-sigtool` | BSD-3-Clause | on-device inspection |
| signature database | ours | data the module reads, not a derivative of it |
| `nemesis` (cloud) | ours | scoring, feed publication |

GPL stays on the mechanism because the two are **separate programs over an
arm's-length netlink interface**, and the module holds **hashes, not
signatures** — it cannot tell you what YouTube is. Moving the matcher into the
kernel would move the product across the boundary with it.

**Known defect:** `aether-fwlogs` declares BSD-3-Clause but links
`libnetfilter_log` (GPL-2.0-or-later). Wrong today. Fix is porting to `libmnl`
(LGPL-2.1+) — the same move the NFQUEUE path needs anyway.

## Data feeds

Of six IP-bearing public feeds screened, **one is usable**: Spamhaus DROP.
Turris DynFW is CC BY-NC-SA (NonCommercial), the abuse.ch family gates
commercial use behind a paid subscription, blocklist.de states no licence.
That is the argument for originating reputation from our own fleet.

## Measured on the BPI-R4

| | |
|---|---|
| Signature load | 1,319 apps / 1,348 rules / 28 merged / **0 refused** |
| DPI classification | **214 µs/flow**, 9.1 µs/packet |
| New-flow rate, baseline | **9,600 conn/s** |
| New-flow rate via NFQUEUE | **6,245 conn/s** (−35%, +56 µs/conn) |
| Reputation applied + verified | 12 sent → 11 held (`auto-merge`) |
| **A packet actually dropped** | counter `1 packet, 84 bytes` |

Classification costs ~4× the userspace crossing, which is why the kernel module
is justified by **failure behaviour** — app filtering surviving a daemon crash
— and not by speed.

**Tests: 413** — 268 sensord, 88 fwlogs, 57 nemesis.
