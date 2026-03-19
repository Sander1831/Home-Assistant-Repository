# RO Pump Complete Setup Guide

> **One-shot install. Follow every step in order and it works.**

---

## 1 — Parts Checklist

| Part | Notes |
|---|---|
| ESP32 WROOM-32 | Any dev-board with standard 38-pin layout |
| FLM19-FW-13 button | 4 wires: 2 × black (switch), 2 × coloured (LED) |
| 1-Channel Relay Module | High-level trigger (blue relay, red board) |
| RO Pump | Runs on the voltage your relay can switch |
| 12 V DC power supply | Powers the button LED and the relay VCC |
| USB cable | For flashing ESPHome firmware to ESP32 |
| Jumper wires | Several male-to-male and male-to-female |

---

## 2 — GPIO Pin Map

| Signal | ESP32 Pin | Direction |
|---|---|---|
| Button switch input | **GPIO32** | INPUT (pull-up, active-LOW) |
| Relay control output | **GPIO12** | OUTPUT (HIGH = pump ON) |

---

## 3 — Wiring Diagram

```
┌─────────────────────────────────────────────────────────┐
│                    POWER SUPPLY  12 V DC                 │
│                   +12 V ────┬─────────────────────────── │
│                   GND  ────┬┤                            │
└────────────────────────────┼┼────────────────────────────┘
                             ││
  FLM19-FW-13 Button         ││
  ┌────────────────┐         ││
  │ LED+  (colour) ├─────────┘│   ← 12 V to LED anode
  │ LED−  (colour) ├──────────┘   ← GND to LED cathode
  │                │
  │ SW    (black)  ├────────────── GPIO32  (ESP32)
  │ SW    (black)  ├────────────── GND     (ESP32)
  └────────────────┘
        (hold 2 / 3 / 5 seconds, then release)

  ESP32 WROOM-32
  ┌──────────────────────┐
  │ GPIO32  ─────────────┼──── Button SW wire (see above)
  │ GPIO12  ─────────────┼──── Relay  IN
  │ GND     ─────────────┼──── Relay  GND  &  Button SW GND
  │ 3V3     ─────────────┼──── Relay  VCC  (3.3 V relay boards)
  └──────────────────────┘     (use 5 V pin if relay needs 5 V)

  Relay Module (high-level trigger)
  ┌──────────────────────────┐
  │ VCC ──── 3V3 or 5V (ESP) │
  │ GND ──── GND      (ESP) │
  │ IN  ──── GPIO12   (ESP) │
  │                          │
  │ COM ──── Pump live wire  │   ← mains or DC pump power
  │ NO  ──── Power supply +  │   ← normally-open: closed when ON
  │ NC  ──── (not used)      │
  └──────────────────────────┘

  RO Pump
  ┌──────────────┐
  │ + ─── Relay NO
  │ − ─── Power supply −
  └──────────────┘
```

> **Mains voltage warning:** If your RO pump runs on AC mains, use a relay rated for mains (≥ 10 A / 250 VAC). Keep mains wiring inside an enclosure. When in doubt, hire a licensed electrician.

---

## 4 — Flash ESPHome Firmware (5 minutes)

### Option A — ESPHome Dashboard (recommended)

1. Open ESPHome Dashboard in your browser.
2. Click **+ New device → Manual**.
3. Give it the name `ro-pump-controller`.
4. Click **Edit** and **replace the entire file** with the contents of `esphome_pump_controller.yaml`.
5. Connect your ESP32 via USB.
6. Click **Install → Plug into this computer**.
7. Wait for compile + upload (~2–3 min). Done.

### Option B — ESPHome CLI

```bash
# Install ESPHome if not already installed
pip install esphome

# Flash the config
esphome run esphome_pump_controller.yaml
```

---

## 5 — Step-by-Step Wiring (in order)

1. **Power everything OFF** before wiring.
2. Connect the two **button switch wires** (black):
   - One to **GPIO32** on the ESP32.
   - One to **GND** on the ESP32.
3. Connect the two **button LED wires** (coloured):
   - Positive → **+12 V** on your 12 V supply.
   - Negative → **GND** on your 12 V supply.
4. Connect the **relay control** side:
   - Relay **VCC** → ESP32 **3V3** (or 5 V if the relay needs 5 V).
   - Relay **GND** → ESP32 **GND**.
   - Relay **IN**  → ESP32 **GPIO12**.
5. Connect the **relay switching** side to the pump power circuit:
   - Relay **COM** → pump power live.
   - Relay **NO**  → power supply positive terminal.
   - (Relay NC is not used.)
6. Connect **pump −** to power supply negative.
7. Double-check every connection, then power on.

---

## 6 — Testing Procedure

Open ESPHome serial monitor (115200 baud) before testing so you can see debug output.

### Test 1 — 2-second hold → 1-minute pump run

1. Hold the button for exactly **2 seconds**, then release.
2. Serial monitor should show:
   ```
   Button PRESSED — hold for 2, 3, or 5 seconds then release
   Button RELEASED after ~2000 ms (2 seconds)
   Hold ≥2 sec → starting 1-minute pump cycle
   >>> PUMP ON  — 1-minute cycle started
   ```
3. The relay should click ON. After **1 minute** it clicks OFF.

### Test 2 — 3-second hold → 3-minute pump run

1. Hold for **3 seconds**, then release.
2. Serial: `Hold ≥3 sec → starting 3-minute pump cycle`
3. Relay ON for **3 minutes**, then OFF.

### Test 3 — 5-second hold → 5-minute pump run

1. Hold for **5 seconds**, then release.
2. Serial: `Hold ≥5 sec → starting 5-minute pump cycle`
3. Relay ON for **5 minutes**, then OFF.

### Test 4 — Short press ignored

1. Tap the button quickly (< 2 seconds).
2. Serial: `Hold too short (X sec) — need ≥2 sec, ignoring`
3. Relay stays OFF. ✅

### Test 5 — Safety timeout

1. Manually turn the relay on via direct GPIO test and leave it.
2. After **6 minutes** the safety watchdog fires:
   ```
   6-minute safety limit reached — forcing pump OFF
   !!! EMERGENCY STOP — pump forced OFF by safety timer
   ```

---

## 7 — Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| Button press does nothing | Wiring issue | Check GPIO32 → one SW wire, GND → other SW wire |
| Wrong duration detected | Button bounce | Increase `delayed_on/off` filter from 50 ms to 100 ms |
| Relay clicks but pump doesn't run | Relay wiring | Confirm COM-NO loop; check pump supply voltage |
| Pump runs continuously | Relay stuck or NO/NC swapped | Swap COM/NO/NC; check relay rated for load |
| ESP32 won't boot | GPIO12 boot issue | GPIO12 has a weak pull-down at boot — ensure relay IN is not pulling it high at startup |
| No serial output | Wrong baud rate | Set monitor to **115200** baud |
| Can't flash | Wrong COM port | Check Device Manager / dmesg for correct port |

---

## 8 — Hold Duration Quick Reference

| Hold time | Pump runs |
|---|---|
| 0–1 sec | Ignored |
| 2 – 2.9 sec | **1 minute** |
| 3 – 4.9 sec | **3 minutes** |
| 5+ sec | **5 minutes** |
| Safety limit | **6 minutes max** (auto-off) |

---

## 9 — LED Behaviour (FLM19-FW-13)

The button LED is powered directly from your 12 V supply — it is always ON when 12 V is present, regardless of pump state. If you want the LED to reflect pump state, wire it through a spare GPIO with an appropriate current-limiting resistor (or a MOSFET for 12 V).

---

*Setup guide for `esphome_pump_controller.yaml` — works with ESP32 WROOM-32.*
