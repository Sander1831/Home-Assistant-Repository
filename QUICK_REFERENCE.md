# QUICK REFERENCE — RO Pump Controller

---

## WIRING DIAGRAM

### Part 1 — Button → ESP32

```
FLM19-FW-13 Button
┌─────────────────────────────┐
│                             │
│  BLACK wire 1  ─────────────┼────► GPIO32  on ESP32
│  BLACK wire 2  ─────────────┼────► GND     on ESP32
│                             │
│  LED+ (coloured) ───────────┼────► +12V    (external supply)
│  LED- (coloured) ───────────┼────► GND     (external supply)
│                             │
└─────────────────────────────┘
```

### Part 2 — ESP32 → Relay Module

```
ESP32                         Relay Module
┌──────────┐                  ┌───────────────┐
│          │                  │               │
│  GPIO12 ─┼──────────────────┼─► IN          │
│  3V3    ─┼──────────────────┼─► VCC         │
│  GND    ─┼──────────────────┼─► GND         │
│          │                  │               │
└──────────┘                  │  COM ─────────┼──► Pump live wire
                              │  NO  ─────────┼──► Power supply +
                              │  NC  (unused) │
                              └───────────────┘
```

### Part 3 — Relay → Pump

```
Power Supply (+) ──► Relay NO
Relay COM        ──► Pump  (+)
Power Supply (-) ──► Pump  (-)
```

### Full picture in one view

```
12V Supply (+) ──────────────────────────────────► Button LED+
12V Supply (-) ──────────────────────────────────► Button LED-
                                                    (LED always on when powered)

Button SW black 1 ──────────────────────────────► GPIO32 (ESP32)
Button SW black 2 ──────────────────────────────► GND    (ESP32)

ESP32 GPIO12 ───────────────────────────────────► Relay IN
ESP32 3V3    ───────────────────────────────────► Relay VCC
ESP32 GND    ───────────────────────────────────► Relay GND

Pump Power (+) ─► Relay NO ──► Relay COM ──► Pump (+)
Pump Power (-) ──────────────────────────────► Pump (-)
```

---

## HOW LONG TO HOLD

| Hold button | Pump runs |
|---|---|
| **2 seconds** then release | 1 minute |
| **3 seconds** then release | 3 minutes |
| **5 seconds** then release | 5 minutes |
| Less than 2 sec | Nothing happens |
| Safety limit | Pump force-off at 6 min |

---

## CODE — PASTE THIS INTO ESPHOME DASHBOARD

Open ESPHome → **+ New device → Manual** → name it `ro-pump-controller` → click **Edit** → **select all and delete** → paste everything below:

```yaml
esphome:
  name: ro-pump-controller
  friendly_name: RO Pump Controller

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
      inverted: true
    id: pump_button
    name: "Pump Button"
    filters:
      - delayed_on: 50ms
      - delayed_off: 50ms
    on_press:
      then:
        - lambda: |-
            id(press_start_ms) = millis();
            ESP_LOGI("button", "Button PRESSED — hold 2/3/5 sec then release");
    on_release:
      then:
        - lambda: |-
            uint32_t held_ms  = millis() - id(press_start_ms);
            uint32_t held_sec = held_ms / 1000;
            ESP_LOGI("button", "Released after %u sec", held_sec);
            if (id(pump_active)) {
              ESP_LOGW("pump", "Pump already running — ignored");
            } else if (held_sec >= 5) {
              ESP_LOGI("pump", "5 sec hold -> 5-minute cycle");
              id(run_pump_5min).execute();
            } else if (held_sec >= 3) {
              ESP_LOGI("pump", "3 sec hold -> 3-minute cycle");
              id(run_pump_3min).execute();
            } else if (held_sec >= 2) {
              ESP_LOGI("pump", "2 sec hold -> 1-minute cycle");
              id(run_pump_1min).execute();
            } else {
              ESP_LOGD("button", "Too short (%u sec) — ignored", held_sec);
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
      - logger.log:
          level: INFO
          format: ">>> PUMP ON — 1 minute"
      - switch.turn_on: pump_relay
      - delay: 1min
      - switch.turn_off: pump_relay
      - lambda: id(pump_active) = false; id(pump_on_start_ms) = 0;
      - logger.log:
          level: INFO
          format: ">>> PUMP OFF — done"

  - id: run_pump_3min
    mode: single
    then:
      - lambda: id(pump_active) = true; id(pump_on_start_ms) = millis();
      - logger.log:
          level: INFO
          format: ">>> PUMP ON — 3 minutes"
      - switch.turn_on: pump_relay
      - delay: 3min
      - switch.turn_off: pump_relay
      - lambda: id(pump_active) = false; id(pump_on_start_ms) = 0;
      - logger.log:
          level: INFO
          format: ">>> PUMP OFF — done"

  - id: run_pump_5min
    mode: single
    then:
      - lambda: id(pump_active) = true; id(pump_on_start_ms) = millis();
      - logger.log:
          level: INFO
          format: ">>> PUMP ON — 5 minutes"
      - switch.turn_on: pump_relay
      - delay: 5min
      - switch.turn_off: pump_relay
      - lambda: id(pump_active) = false; id(pump_on_start_ms) = 0;
      - logger.log:
          level: INFO
          format: ">>> PUMP OFF — done"

  - id: emergency_stop
    then:
      - switch.turn_off: pump_relay
      - lambda: id(pump_active) = false; id(pump_on_start_ms) = 0;
      - logger.log:
          level: WARN
          format: "!!! EMERGENCY STOP — safety limit hit"

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

## FLASH STEPS

1. Paste code above into ESPHome dashboard editor
2. Plug ESP32 into USB
3. Click **Install → Plug into this computer**
4. Wait ~2 min to compile and flash
5. Done — test your button

---

## QUICK TEST

| Action | Expected result |
|---|---|
| Hold 2 sec, release | Relay clicks ON, stays on 1 min, clicks OFF |
| Hold 3 sec, release | Relay ON for 3 min |
| Hold 5 sec, release | Relay ON for 5 min |
| Quick tap (< 2 sec) | Nothing — relay stays OFF |
