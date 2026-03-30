# PUMP CONTROLLER COMPLETE GUIDE

## Hardware

| Component | Detail |
|---|---|
| Microcontroller | ESP32 WROOM-32 |
| Button | FLM19-FW-13 latching push button (12–24 V, IP67) |
| Relay | 1-Channel High-Level Trigger relay module |
| Button GPIO | **GPIO32** |
| Relay GPIO | **GPIO12** |

---

## How It Works

Hold the button down for the required duration, then **release**:

| Hold time | Pump runs |
|---|---|
| 2 – 2.9 sec | **1 minute** |
| 3 – 4.9 sec | **3 minutes** |
| 5+ sec | **5 minutes** |
| < 2 sec | Ignored |
| Safety limit | **6 minutes max (auto-off)** |

---

## Wiring Diagram

```
Button SW wire 1 ──────────────── GPIO32 (ESP32)
Button SW wire 2 ──────────────── GND    (ESP32)
Button LED+      ──────────────── +12 V  (external supply)
Button LED−      ──────────────── GND    (external supply)

ESP32 GPIO12 ──── Relay IN
ESP32 3V3    ──── Relay VCC
ESP32 GND    ──── Relay GND

Relay COM ──── Pump power live
Relay NO  ──── Power supply +
Pump −    ──── Power supply −
```

---

## ESPHome Code

Copy-paste the contents of **`esphome_pump_controller.yaml`** directly into the ESPHome dashboard. The key sections are shown below for reference.

```yaml
esphome:
  name: ro-pump-controller

esp32:
  board: esp32dev
  framework:
    type: arduino

logger:
  level: DEBUG
  baud_rate: 115200

globals:
  - id: press_start_ms
    type: uint32_t
    restore_value: false
    initial_value: '0'
  - id: pump_on_start_ms
    type: uint32_t
    restore_value: false
    initial_value: '0'
  - id: pump_active
    type: bool
    restore_value: false
    initial_value: 'false'

binary_sensor:
  - platform: gpio
    pin:
      number: GPIO32
      mode:
        input: true
        pullup: true
      inverted: true        # LOW = button pressed
    id: pump_button
    name: "Pump Button"
    filters:
      - delayed_on: 50ms    # debounce
      - delayed_off: 50ms
    on_press:
      then:
        - lambda: |-
            id(press_start_ms) = millis();
    on_release:
      then:
        - lambda: |-
            uint32_t held_sec = (millis() - id(press_start_ms)) / 1000;
            if (!id(pump_active)) {
              if      (held_sec >= 5) { id(run_pump_5min).execute(); }
              else if (held_sec >= 3) { id(run_pump_3min).execute(); }
              else if (held_sec >= 2) { id(run_pump_1min).execute(); }
            }

switch:
  - platform: gpio
    pin: GPIO12
    id: pump_relay
    name: "RO Pump Relay"
    restore_mode: ALWAYS_OFF

script:
  - id: run_pump_1min
    mode: single
    then:
      - lambda: id(pump_active) = true; id(pump_on_start_ms) = millis();
      - switch.turn_on: pump_relay
      - delay: 1min
      - switch.turn_off: pump_relay
      - lambda: id(pump_active) = false; id(pump_on_start_ms) = 0;

  - id: run_pump_3min
    mode: single
    then:
      - lambda: id(pump_active) = true; id(pump_on_start_ms) = millis();
      - switch.turn_on: pump_relay
      - delay: 3min
      - switch.turn_off: pump_relay
      - lambda: id(pump_active) = false; id(pump_on_start_ms) = 0;

  - id: run_pump_5min
    mode: single
    then:
      - lambda: id(pump_active) = true; id(pump_on_start_ms) = millis();
      - switch.turn_on: pump_relay
      - delay: 5min
      - switch.turn_off: pump_relay
      - lambda: id(pump_active) = false; id(pump_on_start_ms) = 0;

  - id: emergency_stop
    then:
      - switch.turn_off: pump_relay
      - lambda: id(pump_active) = false; id(pump_on_start_ms) = 0;

interval:
  - interval: 10s
    then:
      - lambda: |-
          if (id(pump_relay).state && id(pump_on_start_ms) > 0) {
            if ((millis() - id(pump_on_start_ms)) / 1000 >= 360) {
              id(emergency_stop).execute();
            }
          }
```

---

## Step-by-Step Installation

1. **Wire components** as shown in the diagram above.
2. Open the **ESPHome Dashboard** in your browser.
3. Click **+ New device → Manual** and name it `ro-pump-controller`.
4. Click **Edit** and replace the file content with `esphome_pump_controller.yaml`.
5. Connect ESP32 via USB and click **Install → Plug into this computer**.
6. Wait ~2–3 minutes for compile and flash.
7. Open the serial monitor (115200 baud) and test.

---

## Testing

1. **Hold 2 seconds → release** — serial shows `1-minute cycle started`, relay clicks ON for 1 min.
2. **Hold 3 seconds → release** — `3-minute cycle started`, relay ON for 3 min.
3. **Hold 5 seconds → release** — `5-minute cycle started`, relay ON for 5 min.
4. **Quick tap** — serial shows `Hold too short`, relay stays OFF.

---

## Troubleshooting

| Problem | Fix |
|---|---|
| No response to button | Check GPIO32 wiring; verify pull-up and inverted: true |
| Wrong timing detected | Increase debounce filter to 100 ms |
| Relay clicks, pump off | Check COM/NO wiring and pump supply |
| Pump runs forever | Safety watchdog fires at 6 min; check interval block |
| GPIO12 relay issue at boot | GPIO12 has boot-time pull-down; relay should be inactive at boot |