# ESP32 + Home Assistant RO Pump Automation — Complete Guide

All code below is **copy-paste ready**. No modifications needed except your Wi-Fi credentials and API keys.

---

## ⚡ QUICK START — Arduino IDE (paste and flash)

> **Use this if you just want to flash the ESP32 right now.**

**File to open:** `ro_pump_esp32.ino` (in this repository)

### 4 steps to flash

1. **Install Arduino IDE** → https://www.arduino.cc/en/software
2. **Add ESP32 board support** — paste this URL in *File → Preferences → Additional boards manager URLs*:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
   Then *Tools → Board → Boards Manager* → search **esp32** → Install
3. **Open the sketch** — *File → Open* → `ro_pump_esp32.ino`
4. **Edit the 4 lines at the top** of the file:
   ```cpp
   const char* WIFI_SSID  = "YOUR_WIFI_SSID";       // ← your WiFi name
   const char* WIFI_PASS  = "YOUR_WIFI_PASSWORD";    // ← your WiFi password
   const char* HA_IP      = "192.168.1.100";         // ← your Home Assistant IP
   const char* HA_TOKEN   = "YOUR_LONG_LIVED_ACCESS_TOKEN"; // ← from HA Profile page
   ```
   Then *Tools → Board → ESP32 Dev Module*, *Tools → Port → your COM port*, click **Upload**.

### Get your HA Long-Lived Access Token
Home Assistant → click your **username** (bottom-left) → **Security** tab → scroll to **Long-lived access tokens** → **Create token** → copy it.

### Add this HA automation (receives pump state from ESP32)

Paste into **Settings → Automations → Edit in YAML**:

```yaml
# ── Sync ESP32 pump state into HA ────────────────────────────────────────────
- alias: "RO Pump — Sync ESP32 State"
  id: ro_pump_sync_esp32_state
  description: Updates HA when ESP32 turns pump on or off
  trigger:
    - platform: state
      entity_id: input_boolean.ro_pump_running
  action:
    - choose:
        - conditions:
            - condition: state
              entity_id: input_boolean.ro_pump_running
              state: "on"
          sequence:
            - service: notify.persistent_notification
              data:
                title: "RO Pump ON"
                message: >
                  Pump started. Duration:
                  {{ state_attr('input_boolean.ro_pump_running', 'duration_seconds') | int }} seconds.
                notification_id: ro_pump_status
        - conditions:
            - condition: state
              entity_id: input_boolean.ro_pump_running
              state: "off"
          sequence:
            - service: persistent_notification.dismiss
              data:
                notification_id: ro_pump_status
  mode: restart
```

Add this helper too (*Settings → Helpers → Create → Toggle*):
- **Name:** `ro_pump_running`
- **Entity ID:** `input_boolean.ro_pump_running`

---

## Table of Contents
1. [Hardware List](#hardware-list)
2. [Wiring Diagram](#wiring-diagram)
3. [ESPHome Configuration (ESP32)](#esphome-configuration-esp32)
4. [Home Assistant Helpers Setup](#home-assistant-helpers-setup)
5. [Home Assistant Automations YAML](#home-assistant-automations-yaml)
6. [configuration.yaml Snippet](#configurationyaml-snippet)
7. [Testing Steps](#testing-steps)

---

## Hardware List

| # | Part | Notes |
|---|------|-------|
| 1 | ESP32 (38-pin DevKit) | Any ESP32 dev board |
| 2 | FLM19-FW-13 push button | 12V–24V LED ring, momentary NO contacts |
| 3 | 1-channel relay module | 5V coil, high-level trigger, optocoupler isolated |
| 4 | RO water pump | 12V or 24V DC |
| 5 | 12V DC power supply | Powers button LED + pump (or separate supplies) |
| 6 | NPN transistor (2N2222 or BC547) | Level-shifts 3.3V → 12V for button LED |
| 7 | Resistors: 1 kΩ × 2, 470 Ω × 1, 10 kΩ × 1 | See wiring diagram (10 kΩ is optional if using ESP32 internal pull-up) |
| 8 | USB 5V supply or 5V rail | Powers ESP32 + relay module VCC |

---

## Wiring Diagram

```
                        12V DC SUPPLY
                        +12V ──────────────────────────────────────┐
                        GND  ──────────────────────┐               │
                                                   │               │
 ┌─────────────────────────────────────────────────┼───────────┐   │
 │                      ESP32 DevKit               │           │   │
 │                                                 │           │   │
 │  3.3V ──[ 10kΩ pull-up ]──┐                     │           │   │
 │                           │                     │           │   │
 │  GPIO18 ──────────────────┤←── BUTTON COM       │           │   │
 │                           │    (FLM19-FW-13)    │           │   │
 │  GND ─────────────────────┴─── BUTTON NO        │           │   │
 │         (button pressed = GPIO18 pulled LOW)    │           │   │
 │                                                 │           │   │
 │  GPIO25 ──[1kΩ]──► BASE (2N2222/BC547)          │           │   │
 │                        │                        │           │   │
 │  GND ──────────── EMITTER                       │           │   │
 │                        │                        │           │   │
 │                    COLLECTOR ──[470Ω]── LED(-)  │           │   │
 │                                         ↑       │           │   │
 │                                    FLM19 LED+───┼───────────┼───┘
 │                                                 │           │
 │  GPIO26 ──────────────────────────────────── IN │           │
 │                                         RELAY   │           │
 │  5V  ──────────────────────────────────── VCC   │           │
 │  GND ───────────────────────────────────── GND ─┘           │
 │                                                             │
 └─────────────────────────────────────────────────────────────┘

RELAY MODULE
  COM ──── PUMP (+)
  NO  ──────────────────────── +12V (from supply)
  (pump GND back to supply GND)

PUMP
  (+) ── RELAY NO
  (-) ── 12V GND

NOTES:
  • ESP32 runs on 5V USB (VIN pin) or 3.3V regulated
  • Relay VCC = 5V  (from ESP32 5V / VIN pin, or separate USB supply)
  • 10kΩ pull-up on GPIO18 is optional — use ESP32 internal pull-up instead
  • 2N2222 transistor drives button LED ring (12V side)
  • GPIO26 = relay control  |  GPIO18 = button input  |  GPIO25 = LED output
```

---

## ESPHome Configuration (ESP32)

Save this file as `ro-pump-controller.yaml` in your ESPHome config folder and flash it to the ESP32.

> **Replace** `YOUR_WIFI_SSID`, `YOUR_WIFI_PASSWORD`, and generate a key with `esphome generate-key`.

```yaml
# ─────────────────────────────────────────────────────────────────────────────
# FILE: ro-pump-controller.yaml
# Flash with: esphome run ro-pump-controller.yaml
# ─────────────────────────────────────────────────────────────────────────────

esphome:
  name: ro-pump-controller
  friendly_name: RO Pump Controller

esp32:
  board: esp32dev
  framework:
    type: arduino

# ── Wi-Fi ────────────────────────────────────────────────────────────────────
wifi:
  ssid: "YOUR_WIFI_SSID"
  password: "YOUR_WIFI_PASSWORD"

  ap:
    ssid: "RO-Pump-Fallback"
    password: "changeme123"

captive_portal:

# ── Logging / API / OTA ──────────────────────────────────────────────────────
logger:

api:
  encryption:
    key: "REPLACE_WITH_YOUR_32_BYTE_BASE64_KEY"

ota:
  - platform: esphome
    password: "YOUR_OTA_PASSWORD"

# ── Global: press counter ────────────────────────────────────────────────────
globals:
  - id: press_count
    type: int
    restore_value: no
    initial_value: '0'

# ── Button input (GPIO18, active-LOW with internal pull-up) ──────────────────
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO18
      mode:
        input: true
        pullup: true
      inverted: true        # LOW = pressed
    name: "Pump Button"
    id: pump_button
    filters:
      - delayed_on: 50ms    # debounce
      - delayed_off: 50ms
    on_press:
      then:
        - globals.set:
            id: press_count
            value: !lambda 'return id(press_count) + 1;'
        - script.execute: detect_press_count

# ── Multi-press detection script ─────────────────────────────────────────────
#   "mode: restart" means every new button press restarts the 600 ms window.
#   After 600 ms of no further presses, the accumulated count is sent to HA.
#   All presses must occur before the 600 ms window closes.
script:
  - id: detect_press_count
    mode: restart
    then:
      - delay: 600ms
      - if:
          condition:
            lambda: 'return id(press_count) >= 3;'
          then:
            - homeassistant.event:
                event: esphome.pump_button_pressed
                data:
                  presses: "3"
          else:
            - if:
                condition:
                  lambda: 'return id(press_count) == 2;'
                then:
                  - homeassistant.event:
                      event: esphome.pump_button_pressed
                      data:
                        presses: "2"
                else:
                  - homeassistant.event:
                      event: esphome.pump_button_pressed
                      data:
                        presses: "1"
      - globals.set:
          id: press_count
          value: '0'

# ── Relay output (GPIO26, high-level trigger) ─────────────────────────────────
switch:
  - platform: gpio
    pin: GPIO26
    name: "RO Pump Relay"
    id: pump_relay
    restore_mode: ALWAYS_OFF
    icon: "mdi:water-pump"

# ── Button LED output (GPIO25 → transistor → 12V LED ring) ───────────────────
output:
  - platform: gpio
    pin: GPIO25
    id: led_output

light:
  - platform: binary
    name: "Button LED"
    id: button_led
    output: led_output
```

---

## Home Assistant Helpers Setup

Add this block to your `configuration.yaml` **OR** create each helper manually in  
**Settings → Devices & Services → Helpers**.

```yaml
# ─────────────────────────────────────────────────────────────────────────────
# Paste into configuration.yaml
# ─────────────────────────────────────────────────────────────────────────────

input_number:
  ro_pump_run_seconds:
    name: "RO Pump Run Duration (seconds)"
    min: 0
    max: 600      # 10 minutes hard ceiling
    step: 1
    mode: box
    unit_of_measurement: "s"
    icon: mdi:timer-outline

timer:
  ro_pump_timer:
    name: "RO Pump Timer"
    duration: "00:01:00"
    restore: true
    icon: mdi:timer
```

After adding, run **Developer Tools → Check configuration** then **Restart Home Assistant**.

---

## Home Assistant Automations YAML

Paste the entire block below into your `automations.yaml`  
(or go to **Settings → Automations → Edit in YAML** and append it).

```yaml
# ─────────────────────────────────────────────────────────────────────────────
# RO PUMP AUTOMATIONS — paste into automations.yaml
# ─────────────────────────────────────────────────────────────────────────────

# ── 1. Respond to button press event from ESP32 ───────────────────────────────
- alias: "RO Pump — Button Pressed"
  id: ro_pump_button_pressed
  description: >
    Receives the ESPHome event, sets the run duration, turns on the pump,
    and starts the countdown timer.
  trigger:
    - platform: event
      event_type: esphome.pump_button_pressed
  action:
    # Set duration based on press count
    - choose:
        - conditions:
            - condition: template
              value_template: "{{ trigger.event.data.presses | int == 1 }}"
          sequence:
            - service: input_number.set_value
              target:
                entity_id: input_number.ro_pump_run_seconds
              data:
                value: 60          # 1 minute

        - conditions:
            - condition: template
              value_template: "{{ trigger.event.data.presses | int == 2 }}"
          sequence:
            - service: input_number.set_value
              target:
                entity_id: input_number.ro_pump_run_seconds
              data:
                value: 180         # 3 minutes

        - conditions:
            - condition: template
              value_template: "{{ trigger.event.data.presses | int >= 3 }}"
          sequence:
            - service: input_number.set_value
              target:
                entity_id: input_number.ro_pump_run_seconds
              data:
                value: 300         # 5 minutes

    # Turn pump ON
    - service: switch.turn_on
      target:
        entity_id: switch.ro_pump_controller_ro_pump_relay

    # Turn button LED ON (visual feedback: pump is running)
    - service: light.turn_on
      target:
        entity_id: light.ro_pump_controller_button_led

    # Start (or restart) the countdown timer
    - service: timer.start
      target:
        entity_id: timer.ro_pump_timer
      data:
        duration: >
          {{ states('input_number.ro_pump_run_seconds') | int }}

  mode: restart   # pressing again mid-run resets the timer

# ── 2. Pump OFF when timer finishes ──────────────────────────────────────────
- alias: "RO Pump — Timer Expired, Pump OFF"
  id: ro_pump_timer_expired
  description: Turns the pump off when the countdown timer reaches zero.
  trigger:
    - platform: event
      event_type: timer.finished
      event_data:
        entity_id: timer.ro_pump_timer
  action:
    - service: switch.turn_off
      target:
        entity_id: switch.ro_pump_controller_ro_pump_relay

    - service: light.turn_off
      target:
        entity_id: light.ro_pump_controller_button_led

  mode: single

# ── 3. Safety shutoff — pump cannot run longer than 6 minutes ────────────────
- alias: "RO Pump — Safety Shutoff (6 min hard limit)"
  id: ro_pump_safety_shutoff
  description: >
    Failsafe: if the pump stays ON for 6 minutes for any reason, force it off
    and cancel the timer.
  trigger:
    - platform: state
      entity_id: switch.ro_pump_controller_ro_pump_relay
      to: "on"
      for:
        minutes: 6
  action:
    - service: switch.turn_off
      target:
        entity_id: switch.ro_pump_controller_ro_pump_relay

    - service: timer.cancel
      target:
        entity_id: timer.ro_pump_timer

    - service: light.turn_off
      target:
        entity_id: light.ro_pump_controller_button_led

    - service: persistent_notification.create
      data:
        title: "⚠️ RO Pump Safety Shutoff"
        message: >
          The pump was forcibly stopped after 6 minutes.
          Check the system before running again.
        notification_id: "ro_pump_safety"

  mode: single
```

---

## configuration.yaml Snippet

If you use ESPHome integration (auto-discovered), no extra `configuration.yaml` changes are needed for the device itself. Just add the helpers section shown above and, optionally, this sensor to track pump run history:

```yaml
# ─────────────────────────────────────────────────────────────────────────────
# Optional — add to configuration.yaml
# ─────────────────────────────────────────────────────────────────────────────

# Helpers (move here if not adding via UI)
input_number:
  ro_pump_run_seconds:
    name: "RO Pump Run Duration (seconds)"
    min: 0
    max: 600
    step: 1
    mode: box
    unit_of_measurement: "s"
    icon: mdi:timer-outline

timer:
  ro_pump_timer:
    name: "RO Pump Timer"
    duration: "00:01:00"
    restore: true
    icon: mdi:timer

# Optional: track how many times the pump ran today
counter:
  ro_pump_daily_runs:
    name: "RO Pump Runs Today"
    initial: 0
    step: 1
    restore: true
    icon: mdi:counter
```

---

## Testing Steps

### Step 1 — Flash the ESP32

```bash
# Install ESPHome if you haven't already
pip install esphome

# Validate config
esphome config ro-pump-controller.yaml

# Flash via USB (first time)
esphome run ro-pump-controller.yaml

# Subsequent updates via Wi-Fi OTA
esphome run ro-pump-controller.yaml --device ro-pump-controller.local
```

### Step 2 — Verify ESP32 connects to Home Assistant

1. Open **Home Assistant → Settings → Devices & Services**
2. You should see **"ESPHome: RO Pump Controller"** auto-discovered
3. Click **Configure** and accept the integration
4. Confirm these entities appear:
   - `switch.ro_pump_controller_ro_pump_relay`
   - `light.ro_pump_controller_button_led`
   - `binary_sensor.ro_pump_controller_pump_button`

### Step 3 — Add helpers

1. Open **Settings → Devices & Services → Helpers → Create Helper**
2. Create **Number** → name it `ro_pump_run_seconds`, min 0, max 600
3. Create **Timer** → name it `ro_pump_timer`, duration 1 minute
4. **OR** paste the `configuration.yaml` block above and restart HA

### Step 4 — Add automations

1. Open **Settings → Automations & Scenes → Automations**
2. Top-right menu → **Edit in YAML**
3. Paste the three automation blocks from Section 5
4. Click **Save**

### Step 5 — Test button presses without the pump

1. Go to **Developer Tools → Events**
2. Event type: `esphome.pump_button_pressed`
3. Event data:
   ```json
   { "presses": "1" }
   ```
4. Click **Fire Event**
5. Verify:
   - `switch.ro_pump_controller_ro_pump_relay` turns ON
   - `timer.ro_pump_timer` starts counting down from 60 s
   - After 60 s, relay turns OFF automatically

Repeat with `"presses": "2"` (3 min) and `"presses": "3"` (5 min).

### Step 6 — Test physical button

| Presses | Expected pump runtime |
|---------|-----------------------|
| 1 press | 1 minute (60 s) |
| 2 presses (within 600 ms window) | 3 minutes (180 s) |
| 3+ presses (within 600 ms window) | 5 minutes (300 s) |

Watch **Developer Tools → States** → filter for `ro_pump` to see all entity states update in real time.

### Step 7 — Test safety shutoff

1. Manually turn on `switch.ro_pump_controller_ro_pump_relay` via HA UI
2. Do NOT cancel the timer
3. Wait 6 minutes
4. Confirm relay turns off and a **persistent notification** appears in HA

### Step 8 — Final wiring check

- With everything wired: press button **once** → relay clicks, pump runs ~60 s, stops
- LED ring on FLM19-FW-13 should glow while pump runs and go off when pump stops

---

## Quick Reference: Entity IDs

| Entity | ID |
|--------|----|
| Relay (pump) | `switch.ro_pump_controller_ro_pump_relay` |
| Button LED | `light.ro_pump_controller_button_led` |
| Button sensor | `binary_sensor.ro_pump_controller_pump_button` |
| Run duration | `input_number.ro_pump_run_seconds` |
| Countdown timer | `timer.ro_pump_timer` |

> If your ESP32 device has a different name in ESPHome, replace `ro_pump_controller` in all entity IDs with the `name:` value you set in the ESPHome config.
