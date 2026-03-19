# Code to Paste

> **Before pasting:** Create two Helpers in HA → Settings → Helpers:
> - Counter → name: `RO Pump Press Counter`  (min 0, max 3, step 1)
> - Toggle  → name: `RO Pump Running`

> **Two entity names to replace in the code below:**
> - `binary_sensor.wifi_button` → your button entity (HA → Settings → Devices)
> - `switch.ro_pump_relay`      → your relay entity  (HA → Settings → Devices)

---

## Automation 1

HA → Settings → Automations → + Create → Edit in YAML → paste → Save

```yaml
alias: "RO Pump - Count Button Press"
trigger:
  - platform: state
    entity_id: binary_sensor.wifi_button   # ← REPLACE
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
              entity_id: switch.ro_pump_relay   # ← REPLACE
          - delay:
              minutes: 1
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay   # ← REPLACE
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
              entity_id: switch.ro_pump_relay   # ← REPLACE
          - delay:
              minutes: 3
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay   # ← REPLACE
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
              entity_id: switch.ro_pump_relay   # ← REPLACE
          - delay:
              minutes: 5
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay   # ← REPLACE
          - service: input_boolean.turn_off
            target:
              entity_id: input_boolean.ro_pump_running
          - service: counter.reset
            target:
              entity_id: counter.ro_pump_press_counter
mode: single
```

---

## Automation 2 — Emergency Stop

```yaml
alias: "RO Pump - Emergency Stop"
trigger:
  - platform: state
    entity_id: binary_sensor.wifi_button   # ← REPLACE
    to: "on"
    for:
      seconds: 3
condition: []
action:
  - service: switch.turn_off
    target:
      entity_id: switch.ro_pump_relay      # ← REPLACE
  - service: input_boolean.turn_off
    target:
      entity_id: input_boolean.ro_pump_running
  - service: counter.reset
    target:
      entity_id: counter.ro_pump_press_counter
mode: single
```
