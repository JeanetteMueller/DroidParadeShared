/*
 * DroidParadeShared - gemeinsame Definitionen für Master und Slave
 * Autorin: Jeanette Müller
 */
#pragma once
#include <stdint.h>

#define DROID_PARADE_MAGIC        0xD401
#define DROID_PARADE_PROTO_VER    1
#define DROID_PARADE_TIMEOUT_MS   1000   // Slave gilt danach als "getrennt"

// Anzahl der Kanäle je Gruppe
#define DP_BODY_LEFT_COUNT     3
#define DP_BODY_RIGHT_COUNT    3
#define DP_BODY_CENTER_COUNT   12
#define DP_DOME_COUNT          6
#define DP_SPECIAL_COUNT       3
#define DP_DOME_ROTATION_MAX   255   // dome_rotation: -255..+255

// light_prop
enum DroidParadeLightProp : uint8_t {
  DP_LIGHT_OFF = 0,
  DP_LIGHT_COLOR,
  DP_LIGHT_RAINBOW
};

// Das über ESP-NOW übertragene Paket (ca. 20 Byte).
// Bool-Gruppen sind als Bitmasken gepackt: Bit 0 = Nr. 1, Bit 1 = Nr. 2, ...
typedef struct __attribute__((packed)) {
  uint16_t magic;         // DROID_PARADE_MAGIC
  uint8_t  version;       // DROID_PARADE_PROTO_VER
  uint16_t sequence;      // fortlaufende Paketnummer
  uint8_t  bodyLeft;      // body_left_1..3
  uint8_t  bodyRight;     // body_right_1..3
  uint16_t bodyCenter;    // body_center_1..12
  uint8_t  bodyArms;      // body_arms (0/1)
  int16_t  domeRotation;  // dome_rotation -255..+255
  uint8_t  dome;          // dome_1..6
  uint8_t  specialAction; // special_action_1..3
  uint8_t  lightProp;     // DroidParadeLightProp
  uint32_t lightColor;    // light_color (z. B. 0xRRGGBB)
  uint8_t  crc;           // CRC-8 über alle vorherigen Bytes
} DroidParadePacket;

// Hilfsfunktionen für Bitmasken (n ist 1-basiert)
static inline bool dpGetBit(uint32_t mask, uint8_t n, uint8_t count) {
  return n >= 1 && n <= count && (mask & (1UL << (n - 1)));
}


// CRC-8 (Poly 0x07) über len Bytes
static inline uint8_t droidParadeCrc8(const uint8_t *data, uint8_t len) {
  uint8_t crc = 0;
  while (len--) {
    crc ^= *data++;
    for (uint8_t i = 0; i < 8; i++) crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
  }
  return crc;
}

static inline void droidParadeSeal(DroidParadePacket &p) {
  p.magic = DROID_PARADE_MAGIC;
  p.version = DROID_PARADE_PROTO_VER;
  p.crc = droidParadeCrc8((const uint8_t *)&p, sizeof(p) - 1);
}

static inline bool droidParadeValid(const DroidParadePacket &p) {
  return p.magic == DROID_PARADE_MAGIC && p.version == DROID_PARADE_PROTO_VER &&
         p.crc == droidParadeCrc8((const uint8_t *)&p, sizeof(p) - 1);
}
