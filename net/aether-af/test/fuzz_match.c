/*
 * Randomised harness for the ring-0 parsers, run under ASan+UBSan.
 *
 * The 185 fixed checks prove the paths they walk. This walks paths nobody
 * wrote a vector for. Two properties matter more than volume:
 *
 *  - every buffer is malloc'd to EXACTLY the length passed in, so a one-byte
 *    overread is a heap-buffer-overflow rather than a quiet read of adjacent
 *    stack that ASan cannot see.
 *  - inputs are seeded with real TLS/HTTP prefixes and then corrupted, so the
 *    parser gets far enough in to reach the interesting code rather than
 *    bailing at byte 0 on random noise.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../src/af_match.h"

static uint64_t s = 0x9E3779B97F4A7C15ULL;
static uint32_t rnd(void) {
    s ^= s << 13; s ^= s >> 7; s ^= s << 17;
    return (uint32_t)(s >> 32);
}

int main(void) {
    char out[256];
    unsigned long iters = 0;

    /* A minimal well-formed TLS ClientHello prefix and an HTTP request line,
     * used as seeds so corruption starts from something parseable. */
    const uint8_t tls_seed[] = {
        0x16,0x03,0x01,0x00,0x30, 0x01,0x00,0x00,0x2c, 0x03,0x03,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0x00, 0x00,0x02,0x00,0x2f, 0x01,0x00, 0x00,0x10,
        0x00,0x00, 0x00,0x0c, 0x00,0x0a, 0x00, 0x00,0x07, 'a','.','b','.','c','o','m'
    };
    const uint8_t http_seed[] =
        "GET / HTTP/1.1\r\nHost: a.b.com\r\nX: y\r\n\r\n";

    for (int round = 0; round < 200000; round++) {
        const uint8_t *seed; size_t seedlen; int is_tls;
        if (rnd() & 1) { seed = tls_seed; seedlen = sizeof(tls_seed); is_tls = 1; }
        else           { seed = http_seed; seedlen = sizeof(http_seed) - 1; is_tls = 0; }

        /* Truncate anywhere, including to zero. */
        uint32_t len = rnd() % (uint32_t)(seedlen + 1);

        /* EXACT-SIZED allocation: this is what makes an overread trap. */
        uint8_t *buf = malloc(len ? len : 1);
        if (!buf) return 1;
        memcpy(buf, seed, len);

        /* Corrupt a few bytes, biased toward the length fields up front. */
        uint32_t muts = rnd() % 5;
        for (uint32_t m = 0; m < muts && len; m++) {
            uint32_t pos = (rnd() & 3) ? (rnd() % (len < 16 ? len : 16)) : (rnd() % len);
            buf[pos] = (uint8_t)rnd();
        }

        /* Vary out_len too, including absurdly small, to exercise the
         * refuse-rather-than-truncate contract. */
        uint32_t olen = (rnd() % 4 == 0) ? (rnd() % 8) + 1 : sizeof(out);
        if (olen > sizeof(out)) olen = sizeof(out);

        if (is_tls) af_extract_sni(buf, len, out, olen);
        else        af_extract_http_host(buf, len, out, olen);

        free(buf);
        iters++;
    }

    printf("fuzz: %lu iterations, no sanitizer trap\n", iters);
    return 0;
}
