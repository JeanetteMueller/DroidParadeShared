# DroidParadeShared

Gemeinsame Basis der **Droid Parade**-Bibliotheken. Wird automatisch von `DroidParadeMaster` und `DroidParadeSlave` eingebunden (`depends=DroidParadeShared`).

**Autorin:** Jeanette Müller · **Version:** 1.0.0 · **Plattform:** ESP32 (Arduino)

## Inhalt
`DroidParadePacket` – das per ESP-NOW übertragene Paket (ca. 20 Byte, gepackt, mit Magic, Version, Sequenznummer und CRC-8):

| Parameter | Typ |
|---|---|
| `body_left_1..3` | bool |
| `body_right_1..3` | bool |
| `body_center_1..12` | bool |
| `body_arms` | bool |
| `dome_rotation` | int, -255..+255 |
| `dome_1..6` | bool |
| `special_action_1..3` | bool |
| `light_prop` | `DP_LIGHT_OFF` / `DP_LIGHT_COLOR` / `DP_LIGHT_RAINBOW` |
| `light_color` | uint32_t (z. B. `0xRRGGBB`) |

Die Bool-Gruppen sind als Bitmasken gepackt (Bit 0 = Nr. 1). Dazu: `droidParadeSeal()` / `droidParadeValid()` und `DROID_PARADE_TIMEOUT_MS`.

Änderungen am Paket immer hier vornehmen und `DROID_PARADE_PROTO_VER` erhöhen; Master und Slave müssen dieselbe Version nutzen.
