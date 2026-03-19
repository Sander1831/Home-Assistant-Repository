# RO PUMP AUTOMATION GUIDE

## Introduction

This guide covers the complete setup for automating a reverse osmosis (RO) pump using an ESP32 WROOM-32, a physical hold-button (FLM19-FW-13), and a 1-channel relay module. No internet or WiFi is required — everything runs locally on the ESP32.

---

## Hardware

| Component | Role |
|---|---|
| ESP32 WROOM-32 | Reads button, controls relay |
| FLM19-FW-13 button | User input (hold 2 / 3 / 5 seconds) |
| 1-Channel Relay Module (HIGH-level trigger) | Switches pump power |
| RO Pump | Load being controlled |

---

## GPIO Pin Map

| Signal | Pin |
|---|---|
| Button switch input | **GPIO32** |
| Relay control output | **GPIO12** |

---

## Wiring Diagram

```
Button SW wire 1 ---------- GPIO32  (ESP32)
Button SW wire 2 ---------- GND     (ESP32)
Button LED+      ---------- +12 V   (external 12 V supply)
Button LED-      ---------- GND     (external supply)

ESP32 GPIO12 ---- Relay IN
ESP32 3V3    ---- Relay VCC
ESP32 GND    ---- Relay GND

Relay COM ---- Pump power live
Relay NO  ---- Power supply positive
Pump -    ---- Power supply negative
```

---

## Hold-Duration Timing

Hold the button down continuously, then release:

| Hold duration | Pump run time |
|---|---|
| 2 - 2.9 seconds | **1 minute** |
| 3 - 4.9 seconds | **3 minutes** |
| 5+ seconds | **5 minutes** |
| < 2 seconds | Ignored (no action) |
| Safety hard-limit | **6 minutes maximum (auto-off)** |

---

## ESPHome Configuration

The complete, production-ready ESPHome configuration is in **`esphome_pump_controller.yaml`**.

Core logic overview:

1. **On button press** - record `millis()` timestamp.
2. **On button release** - calculate `(millis() - press_start) / 1000` seconds held.
3. **Dispatch the matching script** (`run_pump_1min`, `run_pump_3min`, or `run_pump_5min`).
4. **Script turns relay ON**, waits the correct delay, turns relay OFF.
5. **Safety watchdog** (interval: 10 s) forces relay OFF if it has been ON >= 6 minutes.

---

## Installation

1. Wire components as shown above.
2. Open the ESPHome Dashboard -> **+ New device -> Manual** -> name: `ro-pump-controller`.
3. Paste the full content of `esphome_pump_controller.yaml`.
4. Connect ESP32 via USB -> **Install -> Plug into this computer**.
5. Flash completes in ~2-3 minutes.
6. Open serial monitor at **115200 baud** and test.

For full step-by-step instructions and testing procedures see **`RO_PUMP_COMPLETE_SETUP.md`**.

---

## Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| Button does nothing | Wiring error | GPIO32 -> SW wire 1; GND -> SW wire 2 |
| Wrong hold time detected | Contact bounce | Increase `delayed_on/off` to 100 ms |
| Relay clicks but pump off | NO/COM swap | Check relay terminal labels |
| Pump never stops | Safety timer not running | Verify `interval` block is in YAML |
| Short press triggers pump | Debounce too loose | Increase `delayed_on` filter |

---

## Serial Debug Output Examples

```
[I][button]: Button PRESSED — hold for 2, 3, or 5 seconds then release
[I][button]: Button RELEASED after 3021 ms (3 seconds)
[I][pump]: Hold ≥3 sec — starting 3-minute pump cycle
[I][pump]: >>> PUMP ON  — 3-minute cycle started
[I][pump]: >>> PUMP OFF — 3-minute cycle complete
```
