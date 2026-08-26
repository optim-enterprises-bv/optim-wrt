# optim-wrt

OpenWrt package feed for the Aether platform: the device agent, the DPI and
reputation sensor, and their supporting libraries.

If you have an OpenWrt router and want it to appear in the Aether portal, this
is the feed you add.

---

## What is in here

| Package | What it does | Needed for |
|---|---|---|
| `ac-client` | The agent. Speaks USP (TR-369) to the controller over WebSocket. | **Everything.** This is the minimum. |
| `aether-sensord` | Classifies traffic with nDPI, applies reputation and app-block policy, reports flows. | Traffic visibility, parental controls, threat blocking |
| `aether-appdb` | The application signature database `aether-sensord` matches against. | With `aether-sensord` |
| `aether-sigtool` | Builds and inspects that database. Built from the `aether-sensord` Makefile, so selecting that package offers both. | Optional, diagnostics |
| `kmod-aether-af` | Kernel module for in-kernel per-MAC app blocking. Declared as `KernelPackage/aether-af`, so it appears as **kmod-aether-af** and lives under *Kernel modules* in menuconfig, not *Network*. | Enforcement (not just observation) |
| `libndpi` | nDPI, linked by `aether-sensord`. | With `aether-sensord` |
| `nDPId-testing` | Standalone nDPI daemon. | Optional, an alternative flow source |

`ac-client` alone gets a device managed. The rest add traffic intelligence.

---

## Adding the feed

Add this line to `feeds.conf` (or `feeds.conf.default`) in your OpenWrt
buildroot or SDK:

```
src-git optimwrt https://github.com/optim-enterprises-bv/optim-wrt.git
```

Then:

```sh
./scripts/feeds update optimwrt
./scripts/feeds install -a -p optimwrt
```

## Building

Select the packages in `menuconfig` under **Network** (`ac-client`,
`aether-sensord`) and **Libraries** (`libndpi`), then build:

```sh
make menuconfig
make package/feeds/optimwrt/ac-client/compile
make package/feeds/optimwrt/aether-sensord/compile
```

The resulting `.apk` (or `.ipk` on older OpenWrt) files land in
`bin/packages/<arch>/optimwrt/`.

To build a full image with them included, select them as `<*>` in menuconfig
and run `make` as usual.

### Installing on a running device

```sh
scp bin/packages/<arch>/optimwrt/ac-client-*.apk root@192.168.1.1:/tmp/
ssh root@192.168.1.1 'apk add --allow-untrusted /tmp/ac-client-*.apk'
```

> **Note.** OpenWrt's BusyBox has no SFTP server, so `scp` may fail with
> `/usr/libexec/sftp-server: not found`. Stream the file instead:
>
> ```sh
> ssh root@192.168.1.1 'cat > /tmp/ac-client.apk' < ac-client-0.1.0-r16.apk
> ```

---

## Configuring the agent

Configuration lives in `/etc/config/optimacs`. The defaults already point at
the public controller, so in the common case there is **one** setting to check.

### The one thing you must get right

```sh
uci set optimacs.agent.mac_addr='aa:bb:cc:dd:ee:ff'
uci commit optimacs
/etc/init.d/ac-client restart
```

The agent auto-detects this and only needs it set explicitly when detection
fails — it will say so in the log:

```
mac_addr not configured and auto-detection failed.
```

The MAC becomes part of the device's serial (`oui:<OUI>:<mac>`), which is what
you will claim in the portal.

### Settings worth knowing

| Option | Default | Notes |
|---|---|---|
| `ws_url` | `wss://gw.aether-io.com/usp` | The controller. Change only for a private deployment. |
| `ca_file` | `/etc/ssl/certs/ca-certificates.crt` | Must be the **public** CA bundle — the controller's certificate is publicly issued. Pointing this at a private CA breaks the connection at TLS. |
| `status_interval` | `60` | **Do not raise this casually.** See below. |
| `mqtt_url` | *(unset)* | Optional MQTT transport. WebSocket is the supported path. |

#### Why `status_interval` is 60

It doubles as the only periodic traffic on the WebSocket, so it decides whether
the connection survives an idle timeout. Measured against the public endpoint:
Cloudflare closes an idle WebSocket at **~126s**, the ingress behind it at
~300s. At the old default of 300, a board dropped every 128–152s and re-ran the
full onboard each time — about 28 reconnects an hour. At 60 the same board held
one connection indefinitely.

Raise it only if you know every idle timeout on your path.

---

## Getting the device into your account

1. **Install and start the agent** (above). Confirm it connected:

   ```sh
   logread | grep ac-client | tail
   ```

   At startup you will see `WebSocket connection established, TLS handshake
   completed`. That line scrolls out of the ring buffer quickly, so the durable
   sign of a healthy connection is a heartbeat every `status_interval` seconds:

   ```
   ac-client[31631]: WebSocket: Sending status heartbeat (162 bytes)
   ```

   If you see the connection line repeating every couple of minutes rather than
   once, the connection is being reaped and re-established — see
   `status_interval` above.

2. **Find your device serial.** Read it from the agent's own log — it reports
   the endpoint ID it registered with:

   ```sh
   logread | grep -oE 'oui:[0-9A-Fa-f]{6}:[0-9a-f:]{17}' | tail -1
   ```

   ```
   oui:00005A:ea:5e:ca:cf:3f:18
   ```

   If the agent has not logged yet, derive it — the OUI is fixed and the MAC is
   the one auto-detection picks (`br-lan`):

   ```sh
   echo "oui:00005A:$(cat /sys/class/net/br-lan/address)"
   ```

   > Do **not** use `uci get optimacs.agent.mac_addr` for this. It is empty on a
   > normally-configured device, because the MAC is auto-detected rather than
   > stored — you would get `oui:00005A:` and a claim that never matches.

3. **Create an account** at <https://aether-io.com/portal/login> and verify
   your email. Your device list will be empty at this point — that is expected,
   and claiming is what fills it.

4. **Start a claim.** The portal UI does not have a claim button yet, so this
   step is an API call today. Log in and claim:

   ```sh
   BASE=https://gw.aether-io.com
   SERIAL="oui:00005A:aa:bb:cc:dd:ee:ff"    # yours, from step 2

   TOKEN=$(curl -sS -X POST "$BASE/portal/v1/login" \
     -H 'Content-Type: application/json' \
     -d '{"email":"you@example.com","password":"..."}' \
     | sed -n 's/.*"token":"\([^"]*\)".*/\1/p')

   curl -sS -X POST "$BASE/portal/v1/me/devices/claim" \
     -H "Authorization: Bearer $TOKEN" \
     -H 'Content-Type: application/json' \
     -d "{\"serial\":\"$SERIAL\"}"
   ```

   ```json
   {"pending":true,"expires_in":600,"instructions":"Read the claim code from your router..."}
   ```

5. **Read the code off the router** — this is the step that proves the device
   is yours:

   ```sh
   logread | grep aether-claim
   ```

   ```
   aether-claim: claim code is CEAMJP7X -- enter it in the Aether portal to claim this device
   ```

   Or read it directly:

   ```sh
   cat /var/run/aether-claim-code
   ```

6. **Confirm it:**

   ```sh
   curl -sS -X POST "$BASE/portal/v1/me/devices/claim/confirm" \
     -H "Authorization: Bearer $TOKEN" \
     -H 'Content-Type: application/json' \
     -d "{\"serial\":\"$SERIAL\",\"code\":\"CEAMJP7X\"}"
   ```

   ```json
   {"claimed":true,"serial":"oui:00005A:aa:bb:cc:dd:ee:ff"}
   ```

   Your device now appears in `GET /portal/v1/me/devices` and in the portal.

### Why you have to read a code off the router

A serial is not a secret. It is printed on the box, it appears in DHCP logs,
and it is derivable from a MAC address your radio broadcasts in every beacon
frame. If a serial were enough to claim a device, anyone within WiFi range
could bind your router to their account and inherit your WiFi configuration,
your client list and your traffic history.

So the code is delivered to the *device*, and reading it requires SSH or LuCI
access — which is what owning the router actually means. The portal never shows
it to whoever started the claim.

The code expires in **10 minutes** and allows **5 attempts**. Requesting a new
one is safe and resets both. An already-claimed device is refused rather than
transferred.

> **Claiming requires `ac-client` r16 or later.** Older agents do not implement
> the parameter the code is delivered on; the API will tell you so rather than
> sending you to look for a code that was never delivered.

---

## Enabling traffic intelligence

`ac-client` gets the device managed. For flow visibility, app blocking and
threat reputation, add:

```sh
apk add --allow-untrusted /tmp/aether-sensord-*.apk /tmp/aether-appdb-*.apk
/etc/init.d/aether-sensord restart
```

Classification reads packet payload and is consented separately from device
management. `aether-sensord` produces nothing unless it is enabled in
`/etc/config/aether-sensord`:

```
option dpi_enabled   '1'   # classify traffic
option sense_enabled '1'   # report firewall drops for reputation
```

### Two runtime dependencies that fail quietly

`ac-client` pulls in `iw` and `tc`. Without them the radio survey and queue
statistics report nothing and the RF and bufferbloat views in the portal are
silently empty — they were added as explicit dependencies for exactly that
reason. If you build a minimal image, keep them.

---

## Radios are off by default

OpenWrt ships with wireless disabled. A freshly flashed device reports no radio
data until you enable them:

```sh
uci set wireless.radio0.disabled='0'
uci commit wireless
wifi reload
```

This is stock OpenWrt behaviour, not an Aether setting, and it is the usual
reason a new device shows no RF information.

---

## Troubleshooting

**The agent connects and immediately reconnects, repeatedly.**
`status_interval` is too high for an idle timeout on your path. Set it to 60.

**`Post-quantum TLS provider installed successfully` then a crash.**
The binary was built for the wrong CPU. Check you selected the right target.

**The portal says the device is not connected when claiming.**
The claim code travels over the live USP connection, so the device must be
online. Check `logread | grep ac-client`.

**The portal says the device is already claimed.**
It is bound to another account. Devices do not transfer silently; that is
deliberate. Unclaiming is a manual operation today.

**`aether-sensord` runs but reports no flows.**
Check `dpi_enabled '1'` in `/etc/config/aether-sensord`, and that the
`aether-af` kernel module is loaded if you expect blocking rather than
observation only.

---

## Optional: per-device certificates

The agent ships with a bootstrap certificate and connects with it out of the
box — nothing further is required, and this is how a self-installed device
normally runs.

Aether also supports per-device certificates issued over EST
(`/.well-known/est/simpleenroll`, ADR-018). That path needs a single-use,
serial-bound enrolment token, which only a platform administrator can mint, so
it is **not** part of self-service onboarding today. Ask your operator if you
need it.

---

## Licence

Packages here carry their own licences; see each package's `Makefile`. `libndpi`
is LGPL-3.0-or-later, `ac-client` and `aether-sensord` are BUSL-1.1.
