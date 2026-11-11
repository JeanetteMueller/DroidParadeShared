/*
 * DroidParadeShared - gemeinsame Definitionen für Master und Slave
 * Autorin: Jeanette Müller
 */
#pragma once
#include <stdint.h>
#include <string.h>
#include "mbedtls/md.h"

#define DROID_PARADE_MAGIC        0xD401
#define DROID_PARADE_PROTO_VER    2
#define DROID_PARADE_MAC_LEN      8        // gekürzter HMAC-SHA256
#define DROID_PARADE_TIMEOUT_MS   1000   // Slave gilt danach als "getrennt"

// Anzahl der Kanäle je Gruppe
#define DP_BODY_LEFT_COUNT     3
#define DP_BODY_RIGHT_COUNT    3
#define DP_BODY_CENTER_COUNT   12
#define DP_DOME_COUNT          6
#define DP_SPECIAL_COUNT       3
#define DP_DOME_ROTATION_MAX   255   // dome_rotation: -255..+255

// Zugehörigkeit zu einer Parade. Muss auf Master und Slave identisch sein.
//  paradeId: öffentliche Kennung der Parade (trennt parallele Paraden)
//  secret:   geheimer Schlüssel (beliebiger String, mind. 8 Zeichen), signiert jedes Paket
//  channel:  WLAN-Kanal 1..13
struct DroidParadeConfig {
  uint16_t    paradeId;
  const char *secret;
  uint8_t     channel = 1;
};

// light_prop
enum DroidParadeLightProp : uint8_t {
  DP_LIGHT_OFF = 0,
  DP_LIGHT_COLOR,
  DP_LIGHT_RAINBOW
};

// Das über ESP-NOW übertragene Paket (ca. 33 Byte).
// Bool-Gruppen sind als Bitmasken gepackt: Bit 0 = Nr. 1, Bit 1 = Nr. 2, ...
typedef struct __attribute__((packed)) {
  uint16_t magic;         // DROID_PARADE_MAGIC
  uint8_t  version;       // DROID_PARADE_PROTO_VER
  uint16_t paradeId;      // Kennung der Parade
  uint32_t sequence;      // fortlaufende Paketnummer (Replay-Schutz)
  uint8_t  bodyLeft;      // body_left_1..3
  uint8_t  bodyRight;     // body_right_1..3
  uint16_t bodyCenter;    // body_center_1..12
  uint8_t  bodyArms;      // body_arms (0/1)
  int16_t  domeRotation;  // dome_rotation -255..+255
  uint8_t  dome;          // dome_1..6
  uint8_t  specialAction; // special_action_1..3
  uint8_t  lightProp;     // DroidParadeLightProp
  uint32_t lightColor;    // light_color (z. B. 0xRRGGBB)
  uint8_t  mac[DROID_PARADE_MAC_LEN]; // HMAC-SHA256 (gekürzt) über alle vorherigen Bytes
} DroidParadePacket;

// Hilfsfunktionen für Bitmasken (n ist 1-basiert)
static inline bool dpGetBit(uint32_t mask, uint8_t n, uint8_t count) {
  return n >= 1 && n <= count && (mask & (1UL << (n - 1)));
}


// Gekürzter HMAC-SHA256 über das Paket (ohne mac-Feld)
static inline void droidParadeMac(const DroidParadePacket &p, const char *secret, uint8_t out[DROID_PARADE_MAC_LEN]) {
  uint8_t full[32] = {0};
  mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
                  (const uint8_t *)secret, strlen(secret),
                  (const uint8_t *)&p, sizeof(p) - DROID_PARADE_MAC_LEN, full);
  memcpy(out, full, DROID_PARADE_MAC_LEN);
}

static inline void droidParadeSeal(DroidParadePacket &p, const char *secret) {
  p.magic = DROID_PARADE_MAGIC;
  p.version = DROID_PARADE_PROTO_VER;
  droidParadeMac(p, secret, p.mac);
}

// Prüft Magic, Version und Signatur (konstante Vergleichszeit)
static inline bool droidParadeValid(const DroidParadePacket &p, const char *secret) {
  if (p.magic != DROID_PARADE_MAGIC || p.version != DROID_PARADE_PROTO_VER) return false;
  uint8_t m[DROID_PARADE_MAC_LEN];
  droidParadeMac(p, secret, m);
  uint8_t diff = 0;
  for (uint8_t i = 0; i < DROID_PARADE_MAC_LEN; i++) diff |= m[i] ^ p.mac[i];
  return diff == 0;
}
