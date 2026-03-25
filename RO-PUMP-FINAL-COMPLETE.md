# RO PUMP FINAL — COMPLETE

## What You Have
- **FLM19-FW-13** push button (12V–24V, LED indicator, dry-contact output)
- **1-channel relay module** (5V logic, HIGH-level trigger)
- **Water pump**
- **Home Assistant OS** (Raspberry Pi or similar)

---

## 1. WIRING DIAGRAM

```
┌─────────────────────────────────────────────────────────────┐
│                    12V POWER SUPPLY                         │
│         +12V ──────┬─────────────────────────────────────── │
│                    │                                        │
│           FLM19-FW-13 BUTTON                                │
│         +12V ──── [LED+]  (button LED power)                │
│         GND  ──── [LED-]  (button LED ground)               │
│                                                             │
│         [COM] ──── Raspberry Pi GND                         │
│         [NO]  ──── Raspberry Pi GPIO17  (button input)      │
│                    (internal pull-up enabled in HA)         │
│                    button pressed = GPIO17 pulled LOW        │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│               1-CHANNEL RELAY MODULE (5V)                   │
│         VCC  ──── Raspberry Pi 5V                           │
│         GND  ──── Raspberry Pi GND                          │
│         IN   ──── Raspberry Pi GPIO27  (relay control)      │
│                    HIGH = relay ON = pump runs               │
│                                                             │
│         COM  ──── Pump power (Live / +)                     │
│         NO   ──── Mains Live IN (from wall / supply)        │
│         (Neutral wire connects directly to pump)            │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                      PUMP WIRING                            │
│         Relay COM  ──── Pump (+) / Live                     │
│         Relay NO   ──── Mains Live IN                       │
│         Neutral    ──── Pump (-) / Neutral (direct)         │
└─────────────────────────────────────────────────────────────┘
```

> **Safety:** If your pump runs on mains AC, have a licensed electrician wire the mains side. The relay switches the Live wire only.

---

## 2. HELPER ENTITIES — ADD TO `configuration.yaml`

Paste this into your `configuration.yaml` file (or in a separate `helpers.yaml` that you include):

```yaml
input_number:
  ro_pump_press_count:
    name: RO Pump Press Count
    min: 0
    max: 10
    step: 1
    mode: box

timer:
  ro_pump_press_window:
    name: RO Pump Press Window
    duration: "00:00:02"

  ro_pump_run_timer:
    name: RO Pump Run Timer
    duration: "00:01:00"
```

---

## 3. AUTOMATION — COPY INTO `automations.yaml`

Paste all three automations below into your `automations.yaml` file.

```yaml
# ── AUTOMATION 1: COUNT BUTTON PRESSES ───────────────────────
- id: ro_pump_count_press
  alias: "RO Pump - Count Button Press"
  description: "Increments press counter and resets the 2-second press window each time the button is pressed."
  trigger:
    - platform: state
      entity_id: binary_sensor.ro_pump_button
      from: "off"
      to: "on"
  action:
    - service: input_number.set_value
      target:
        entity_id: input_number.ro_pump_press_count
      data:
        value: "{{ (states('input_number.ro_pump_press_count') | int) + 1 }}"
    - service: timer.start
      target:
        entity_id: timer.ro_pump_press_window
      data:
        duration: "00:00:02"
  mode: queued

# ── AUTOMATION 2: START PUMP WHEN PRESS WINDOW CLOSES ────────
- id: ro_pump_start
  alias: "RO Pump - Start After Press Window"
  description: "Starts pump when the 2-second press window expires. 1 press=1min, 2 presses=3min, 3+ presses=5min."
  trigger:
    - platform: event
      event_type: timer.finished
      event_data:
        entity_id: timer.ro_pump_press_window
  action:
    - choose:
        - conditions:
            - condition: template
              value_template: "{{ states('input_number.ro_pump_press_count') | int == 1 }}"
          sequence:
            - service: switch.turn_on
              target:
                entity_id: switch.ro_pump_relay
            - service: timer.start
              target:
                entity_id: timer.ro_pump_run_timer
              data:
                duration: "00:01:00"

        - conditions:
            - condition: template
              value_template: "{{ states('input_number.ro_pump_press_count') | int == 2 }}"
          sequence:
            - service: switch.turn_on
              target:
                entity_id: switch.ro_pump_relay
            - service: timer.start
              target:
                entity_id: timer.ro_pump_run_timer
              data:
                duration: "00:03:00"

        - conditions:
            - condition: template
              value_template: "{{ states('input_number.ro_pump_press_count') | int >= 3 }}"
          sequence:
            - service: switch.turn_on
              target:
                entity_id: switch.ro_pump_relay
            - service: timer.start
              target:
                entity_id: timer.ro_pump_run_timer
              data:
                duration: "00:05:00"
    - service: input_number.set_value
      target:
        entity_id: input_number.ro_pump_press_count
      data:
        value: 0
  mode: single

# ── AUTOMATION 3: AUTO-STOP PUMP WHEN TIMER EXPIRES ──────────
- id: ro_pump_auto_stop
  alias: "RO Pump - Auto Stop"
  description: "Turns off pump when run timer finishes. Pump cannot run indefinitely."
  trigger:
    - platform: event
      event_type: timer.finished
      event_data:
        entity_id: timer.ro_pump_run_timer
  action:
    - service: switch.turn_off
      target:
        entity_id: switch.ro_pump_relay
    - service: input_number.set_value
      target:
        entity_id: input_number.ro_pump_press_count
      data:
        value: 0
  mode: single
```

---

## 4. HOW IT WORKS

| Button Presses | Pump Runtime |
|:--------------:|:------------:|
| 1 press        | 1 minute     |
| 2 presses      | 3 minutes    |
| 3+ presses     | 5 minutes    |

- You have **2 seconds** after the first press to add more presses.
- After the window closes, the pump starts automatically.
- When the timer expires, the pump turns off automatically — it **cannot run forever**.

---

## 5. SETUP STEPS

### Step 1 — Add helpers
1. Open Home Assistant → **Settings → Add-ons → File editor** (or use SSH).
2. Open `configuration.yaml`.
3. Paste the **Helper Entities** block from Section 2.
4. Save the file.

### Step 2 — Add automations
1. Open `automations.yaml`.
2. Paste all three automations from Section 3.
3. Save the file.

### Step 3 — Restart Home Assistant
1. **Settings → System → Restart**.
2. Wait for it to come back up.

### Step 4 — Wire hardware
1. Follow the wiring diagram in Section 1.
2. Connect FLM19-FW-13 contacts (COM + NO) → RPi GPIO17 + GND.
3. Connect relay module (VCC, GND, IN) → RPi 5V, GND, GPIO27.
4. Connect relay output (COM + NO) → pump power wiring.

### Step 5 — Register GPIO entities in HA
Add this to `configuration.yaml` (adjust pin numbers to match your actual wiring):

```yaml
binary_sensor:
  - platform: rpi_gpio
    ports:
      17: ro_pump_button
    pull_mode: UP
    invert_logic: true

switch:
  - platform: rpi_gpio
    ports:
      27: ro_pump_relay
```

Restart HA after adding this.

---

## 6. TESTING

1. **Test button detection:**
   - Go to **Developer Tools → States**.
   - Find `binary_sensor.ro_pump_button`.
   - Press the physical button — state should flip from `off` → `on`.

2. **Test 1 press (1 minute):**
   - Press button once.
   - Check `switch.ro_pump_relay` turns ON.
   - After 1 minute, check it turns OFF automatically.

3. **Test 2 presses (3 minutes):**
   - Press button twice within 2 seconds.
   - Check `switch.ro_pump_relay` turns ON.
   - After 3 minutes, check it turns OFF automatically.

4. **Test 3 presses (5 minutes):**
   - Press button three times within 2 seconds.
   - Check `switch.ro_pump_relay` turns ON.
   - After 5 minutes, check it turns OFF automatically.

5. **Test auto-stop (safety):**
   - In **Developer Tools → Services**, call `timer.finish` on `timer.ro_pump_run_timer`.
   - Pump should turn off immediately.

---

## ENTITY NAMES USED

| Entity | What it is |
|--------|------------|
| `binary_sensor.ro_pump_button` | FLM19-FW-13 button input |
| `switch.ro_pump_relay` | Relay module (controls pump) |
| `input_number.ro_pump_press_count` | Press counter helper |
| `timer.ro_pump_press_window` | 2-second multi-press window |
| `timer.ro_pump_run_timer` | Pump run countdown timer |
