# RO Pump — Copy-Paste Code

> Just paste these into Home Assistant and change the two entity names to match your devices.  
> Full setup instructions (wiring, helpers, testing): [RO-PUMP-AUTOMATION-SETUP.md](RO-PUMP-AUTOMATION-SETUP.md)

---

## Before You Paste — 2 things to replace

| Placeholder | Replace with | Where to find it |
|-------------|-------------|-----------------|
| `binary_sensor.wifi_button` | Your button's entity ID | HA → Settings → Devices & Services → find your button |
| `switch.ro_pump_relay` | Your relay's entity ID | HA → Settings → Devices & Services → find your relay |

---

## Step 1 — Create the Helpers (do this once in HA UI)

1. Go to **Settings → Devices & Services → Helpers → + Create Helper**
2. Create a **Counter** named `RO Pump Press Counter` (min 0, max 3, step 1)
3. Create a **Toggle** named `RO Pump Running`

---

## Step 2 — Paste the Automations

Go to **Settings → Automations → + Create Automation → Edit in YAML**, paste each block below.

---

### Automation 1 — Button Press Counter & Timer

```yaml
alias: "RO Pump - Count Button Press"
description: "Counts how many times the button is pressed and picks the timer"
trigger:
  - platform: state
    entity_id: binary_sensor.wifi_button   # ← REPLACE with your button entity
    to: "on"
condition:
  - condition: state
    entity_id: input_boolean.ro_pump_running
    state: "off"
action:
  - service: counter.increment
    target:
      entity_id: counter.ro_pump_press_counter
  - delay:
      seconds: 4
  - choose:
      - conditions:
          - condition: state
            entity_id: counter.ro_pump_press_counter
            state: "1"
        sequence:
          - service: input_boolean.turn_on
            target:
              entity_id: input_boolean.ro_pump_running
          - service: switch.turn_on
            target:
              entity_id: switch.ro_pump_relay  # ← REPLACE with your relay entity
          - delay:
              minutes: 1
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay  # ← REPLACE with your relay entity
          - service: input_boolean.turn_off
            target:
              entity_id: input_boolean.ro_pump_running
          - service: counter.reset
            target:
              entity_id: counter.ro_pump_press_counter
      - conditions:
          - condition: state
            entity_id: counter.ro_pump_press_counter
            state: "2"
        sequence:
          - service: input_boolean.turn_on
            target:
              entity_id: input_boolean.ro_pump_running
          - service: switch.turn_on
            target:
              entity_id: switch.ro_pump_relay  # ← REPLACE with your relay entity
          - delay:
              minutes: 3
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay  # ← REPLACE with your relay entity
          - service: input_boolean.turn_off
            target:
              entity_id: input_boolean.ro_pump_running
          - service: counter.reset
            target:
              entity_id: counter.ro_pump_press_counter
      - conditions:
          - condition: state
            entity_id: counter.ro_pump_press_counter
            state: "3"
        sequence:
          - service: input_boolean.turn_on
            target:
              entity_id: input_boolean.ro_pump_running
          - service: switch.turn_on
            target:
              entity_id: switch.ro_pump_relay  # ← REPLACE with your relay entity
          - delay:
              minutes: 5
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay  # ← REPLACE with your relay entity
          - service: input_boolean.turn_off
            target:
              entity_id: input_boolean.ro_pump_running
          - service: counter.reset
            target:
              entity_id: counter.ro_pump_press_counter
mode: single
```

---

### Automation 2 — Emergency Stop

Hold the button for 3 seconds to stop the pump immediately.

```yaml
alias: "RO Pump - Emergency Stop"
description: "Hold the button 3 seconds to stop the pump immediately"
trigger:
  - platform: state
    entity_id: binary_sensor.wifi_button   # ← REPLACE with your button entity
    to: "on"
    for:
      seconds: 3
condition: []
action:
  - service: switch.turn_off
    target:
      entity_id: switch.ro_pump_relay      # ← REPLACE with your relay entity
  - service: input_boolean.turn_off
    target:
      entity_id: input_boolean.ro_pump_running
  - service: counter.reset
    target:
      entity_id: counter.ro_pump_press_counter
mode: single
```

---

## What each press does

```
Press 1×  →  Pump runs 1 minute
Press 2×  →  Pump runs 3 minutes
Press 3×  →  Pump runs 5 minutes
Hold 3s   →  Pump stops immediately
```
