# RO Pump Automation Setup — One-Button Hold-Duration Timer

> **Beginner-friendly guide** — no coding or electronics experience required!

---

## 📋 Table of Contents
1. [How It Works](#how-it-works)
2. [Shopping List](#shopping-list)
3. [System Overview Diagram](#system-overview-diagram)
4. [Component Diagrams](#component-diagrams)
5. [Wiring Diagrams](#wiring-diagrams)
6. [Step-by-Step Wiring Instructions](#step-by-step-wiring-instructions)
7. [Home Assistant Setup](#home-assistant-setup)
8. [Automation Code](#automation-code)
9. [Testing the System](#testing-the-system)
10. [Troubleshooting](#troubleshooting)

---

## How It Works

Hold the button for a set number of seconds and the pump runs for the matching duration. Release the button to start the timer:

```
╔══════════════════════════════════════════════════════╗
║         ONE-BUTTON HOLD-DURATION TIMER               ║
╠══════════════════════════════════════════════════════╣
║  Hold 2 sec  ──►  Release  ──►  Pump runs 1 minute   ║
║  Hold 3 sec  ──►  Release  ──►  Pump runs 3 minutes  ║
║  Hold 5 sec  ──►  Release  ──►  Pump runs 5 minutes  ║
╚══════════════════════════════════════════════════════╝
```

**Press and hold the button, watch the LED light up, then release — Home Assistant measures how long you held it and picks the right timer automatically.**

---

## Shopping List

| # | Item | Notes |
|---|------|-------|
| 1 | **FLM19-FW-13 Hardwired Push Button Switch** | IP67 waterproof, 19mm latching button, 12V–24V, rated switching current 15A–20A, 4-wire (2 black = switch, 2 colored = LED) |
| 2 | **DieseRC 1-Channel WiFi Relay Module** | The relay that switches the pump on/off |
| 3 | **RO Pump (your existing pump)** | Standard reverse osmosis pump |
| 4 | **5V or 12V DC Power Supply** | To power the relay module and button LED (check relay label) |
| 5 | **Home Assistant device** | Raspberry Pi, NUC, or any computer running HA |
| 6 | **Electrical wire** | 18 AWG for pump wiring; 1.5 mm² (15 AWG) for button wiring |
| 7 | **Wire connectors / terminal blocks** | For safe connections |
| 8 | **Screwdriver (small flathead)** | For relay terminal screws |

> ⚠️ **Safety Note:** The RO pump typically runs on mains voltage (120V or 240V). If you are unsure about wiring mains voltage, hire a licensed electrician.

---

## System Overview Diagram

This diagram shows how all the pieces connect at a high level:

```
┌─────────────────────────────────────────────────────────────┐
│                    YOUR HOME WIFI NETWORK                   │
│                                                             │
│                           ┌──────────────────────────────┐  │
│                           │                              │  │
│                           │     HOME ASSISTANT           │  │
│                           │     (your smart home hub)   │  │
│                           │                              │  │
│                           └──────────────┬───────────────┘  │
│                                          │ WiFi command      │
│                                          ▼                   │
│                           ┌──────────────────────────────┐  │
│                           │   DieseRC WiFi Relay Module  │  │
│                           │   (receives ON/OFF commands) │  │
│                           └──────────────┬───────────────┘  │
│                                          │                   │
└──────────────────────────────────────────┼───────────────────┘
                                           │ switches power
                                           ▼
                           ┌──────────────────────────────┐
                           │        RO PUMP MOTOR         │
                           │  (turns on / turns off)      │
                           └──────────────────────────────┘

     ┌───────────────────────────────┐
     │  FLM19-FW-13 BUTTON (wired)  │
     │  [2 black wires → relay IN]   │──────► Relay controller input
     │  [2 colored wires → LED pwr]  │        (binary_sensor in HA)
     └───────────────────────────────┘
```

**In plain English:**
1. You press and hold the button → Button LED lights up, switch closes
2. You release the button → Home Assistant measures how long you held it
3. Home Assistant picks the right timer (1, 3, or 5 min) and tells the WiFi relay to turn ON
4. The relay switches the power to your pump ON
5. When the timer runs out, Home Assistant tells the relay to turn OFF
6. Pump turns OFF!

---

## Component Diagrams

### 📦 FLM19-FW-13 Hardwired Push Button (Front View)

```
         ┌─────────────────┐
         │  ┌───────────┐  │
         │  │           │  │
         │  │  PRESS &  │  │   ← Latching button (each press toggles ON/OFF)
         │  │   HOLD    │  │
         │  │           │  │
         │  └───────────┘  │
         │                 │
         │  [LED ring]     │   ← LED lights up when switch is ON
         │                 │
         │  ~~~~ IP67 ~~~~ │   ← IP67 waterproof rating
         └─────────────────┘

         4 wires exit the back:
         ● Black wire 1  ┐  Switch control
         ● Black wire 2  ┘  (connect to relay input terminals)
         ● Colored wire 1 ┐  LED indicator
         ● Colored wire 2 ┘  (non-polarized, any polarity works)

         Specs: 19 mm, 12V–24V, 15A–20A, 100,000 cycle service life
```

---

### 📦 DieseRC 1-Channel WiFi Relay Module (Top View)

```
   ┌──────────────────────────────────────────────────┐
   │              DieseRC WiFi RELAY MODULE            │
   │                                                  │
   │  ┌─────────────────────────────────────────────┐ │
   │  │  [WiFi antenna]          [LED indicator]    │ │
   │  │                                             │ │
   │  │              [Relay coil inside]            │ │
   │  └─────────────────────────────────────────────┘ │
   │                                                  │
   │  POWER INPUT          RELAY OUTPUT TERMINALS     │
   │  ┌──────────┐         ┌─────────────────────┐   │
   │  │  VCC  │GND│        │  COM  │  NO  │  NC  │   │
   │  └──────────┘         └─────────────────────┘   │
   │   (+)   (-)              ↑       ↑       ↑      │
   └──────────────────────────┼───────┼───────┼──────┘
                              │       │       │
                           Common  Normally  Normally
                           (always  Open    Closed
                           connected) (off) (on by default)
```

**Terminal Legend:**
- `VCC` = Positive power input (+5V or +12V — check your relay's label)
- `GND` = Negative / Ground (-)
- `COM` = Common terminal (always connected — pump wire goes here)
- `NO` = Normally Open (pump is OFF until relay switches ON — use this one!)
- `NC` = Normally Closed (pump is ON by default — do NOT use this)

> ✅ **You will use: COM and NO** — this means the pump stays OFF until Home Assistant turns it on.

---

### 📦 Power Supply Connections

```
   WALL OUTLET (120V/240V AC)
         │
         │  AC Power cord
         ▼
   ┌─────────────┐
   │  AC/DC      │
   │  Adapter    │   (5V or 12V DC output — match your relay's requirement)
   └──────┬──────┘
          │
     ┌────┴────┐
     │         │
   (+) Red   (-) Black
     │         │
     ▼         ▼
   [VCC]     [GND]   ← On the relay module power input terminals
```

---

## Wiring Diagrams

### Complete Wiring Diagram

```
     ╔══════════════════════════════════════════════╗
     ║   FLM19-FW-13 BUTTON                         ║
     ║                                              ║
     ║  [Black wire 1] ─────────────────────────────╫──► Relay controller
     ║  [Black wire 2] ─────────────────────────────╫──► input terminal (GND)
     ║                                              ║
     ║  [Colored wire 1] ──────────────────┐        ║
     ║  [Colored wire 2] ──────────────┐   │        ║
     ╚═════════════════════════════════╪═══╪════════╝
                                       │   │
                         LED wire A    ┘   └  LED wire B  (non-polarized — either way)
                                       │   │
                     ╔═════════════════╪═══╪═════════════════════╗
                     ║     POWER SUPPLY (12V DC)                 ║
                     ║                                           ║
                     ║  [+] Red ─────────────────────────┐       ║
                     ║  [-] Black ───────────────────┐   │       ║
                     ╚══════════════════════════════╪═══╪═══════╝
                                                    │   │
                                                    ▼   ▼
                     ╔══════════════════════════════════════╗
                     ║     DieseRC WiFi RELAY MODULE        ║
                     ║                                      ║
                     ║  ┌─────┐ ┌─────┐  ┌────┬────┬────┐  ║
                     ║  │ VCC │ │ GND │  │COM │ NO │ NC │  ║
                     ║  └──┬──┘ └──┬──┘  └──┬─┴──┬─┴────┘  ║
                     ║     │      │         │    │          ║
                     ╚═════╪══════╪═════════╪════╪══════════╝
                           │      │         │    │
                  Power(+) ┘      │ GND(-)  │    │
                                  └─────────┘    │
                                  (ground shared)│
                                                 │
     ╔═══════════════════╗                       │
     ║   WALL OUTLET     ║                       │
     ║   (Mains power)   ║                       │
     ║                   ║        ┌──────────────┘
     ║  LIVE ────────────╫────────┤ COM  (relay common)
     ║                   ║        │
     ║  NEUTRAL ─────────╫────────┼──────────────────────┐
     ║                   ║        │                      │
     ║  EARTH/GND ───────╫──┐     │ NO  (normally open)  │
     ║                   ║  │     └──────────────────┐   │
     ╚═══════════════════╝  │                        │   │
                            │     ╔══════════════════╪═══╪═══╗
                            │     ║   RO PUMP MOTOR  │   │   ║
                            │     ║                  │   │   ║
                            │     ║  LIVE ───────────┘   │   ║
                            │     ║  NEUTRAL─────────────┘   ║
                            └─────╫──EARTH/GND               ║
                                  ╚═══════════════════════════╝
```
---

### Simplified Wiring (Easy-to-Read Version)

```
STEP 1: Power the Relay Module
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Power Supply (+) ──── Relay VCC terminal
  Power Supply (-) ──── Relay GND terminal


STEP 2: Connect the Pump to the Relay
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Wall outlet LIVE wire ──── Relay COM terminal
  Relay NO terminal     ──── Pump LIVE wire
  Wall outlet NEUTRAL   ──── Pump NEUTRAL wire (direct, bypasses relay)
  Earth/Ground          ──── Pump EARTH wire  (direct, bypasses relay)


STEP 3: Wire the FLM19-FW-13 Button
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Black wire 1   ──── Relay controller input terminal (signal)
  Black wire 2   ──── Relay controller GND terminal
  Colored wire 1 ──── Power supply (+) for LED  (non-polarized)
  Colored wire 2 ──── Power supply (-) for LED  (either wire to either terminal)


STEP 4: Relay Module WiFi (no wires needed!)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Just pair it with your WiFi network using the app — it connects wirelessly!
```
---

### Relay "Normally Open" Explained

```
  ┌─────────────────────────────────────────────────────┐
  │                                                     │
  │   RELAY OFF (pump is OFF):                          │
  │                                                     │
  │   COM ────X────── NO     ← circuit is OPEN (broken) │
  │                          ← No power flows to pump   │
  │   COM ─────────── NC     ← circuit is CLOSED        │
  │                                                     │
  ├─────────────────────────────────────────────────────┤
  │                                                     │
  │   RELAY ON (pump is ON):                            │
  │                                                     │
  │   COM ──────────── NO    ← circuit is CLOSED        │
  │                          ← Power flows to pump! ⚡  │
  │   COM ────X──────── NC   ← circuit is OPEN (broken) │
  │                                                     │
  └─────────────────────────────────────────────────────┘
```

---

### One-Button Hold-Duration Timer Flow

```
  ┌─────────────────────────────────────────────────────────────┐
  │                  BUTTON HOLD FLOW                           │
  └─────────────────────────────────────────────────────────────┘

  Press & hold button (LED lights up)
       │
       ▼
  ┌────────────────────────────────────────────────────────────┐
  │  Release button                                            │
  │                                                            │
  │  Hold was ≥ 5 sec? ─ YES ──► Set timer: 5 MINUTES         │
  │       │                            │                       │
  │       NO                           │                       │
  │       │                            │                       │
  │  Hold was ≥ 3 sec? ─ YES ──► Set timer: 3 MINUTES         │
  │       │                            │                       │
  │       NO                           │                       │
  │       │                            │                       │
  │  Hold was ≥ 2 sec? ─ YES ──► Set timer: 1 MINUTE          │
  │       │                            │                       │
  │       NO  ──► Ignored (too short)  │                       │
  └────────────────────────────────────┼───────────────────────┘
                │
                ▼
  ┌─────────────────────────────────────────────────────────────┐
  │  Home Assistant sends ON command to WiFi Relay              │
  └──────────────────────────┬──────────────────────────────────┘
                             │
                             ▼
  ┌─────────────────────────────────────────────────────────────┐
  │  Relay closes → Power flows → RO Pump turns ON  ⚡          │
  └──────────────────────────┬──────────────────────────────────┘
                             │
                             ▼ (wait for timer to count down)
  ┌─────────────────────────────────────────────────────────────┐
  │  Timer runs out (1, 3, or 5 minutes)                        │
  └──────────────────────────┬──────────────────────────────────┘
                             │
                             ▼
  ┌─────────────────────────────────────────────────────────────┐
  │  Home Assistant sends OFF command to WiFi Relay             │
  └──────────────────────────┬──────────────────────────────────┘
                             │
                             ▼
  ┌─────────────────────────────────────────────────────────────┐
  │  Relay opens → Power stops → RO Pump turns OFF  ■           │
  └─────────────────────────────────────────────────────────────┘
```
---

## Step-by-Step Wiring Instructions

### ⚠️ Safety First!

```
  ┌─────────────────────────────────────────────────────┐
  │  🔴 ALWAYS unplug the pump before wiring            │
  │  🔴 ALWAYS unplug the power supply before wiring    │
  │  🔴 If unsure about mains voltage, call electrician │
  │  🟢 Low-voltage (relay power) wiring is safe to do  │
  └─────────────────────────────────────────────────────┘
```

---

### Step 1 — Wire the FLM19-FW-13 Button

```
  ┌─────────────────────────────────────────────────────┐
  │                                                     │
  │  Your FLM19-FW-13 has 4 wires:                     │
  │                                                     │
  │  ● Black wire 1  ┐  Switch control wires            │
  │  ● Black wire 2  ┘  (these connect the switch loop) │
  │                                                     │
  │  ● Colored wire 1 ┐  LED indicator wires            │
  │  ● Colored wire 2 ┘  (non-polarized — either way)  │
  │                                                     │
  │  SWITCH WIRES (2 black wires):                      │
  │                                                     │
  │  1. Connect Black wire 1 to the relay controller    │
  │     signal/input terminal (or GPIO binary sensor)   │
  │                                                     │
  │  2. Connect Black wire 2 to the relay controller    │
  │     GND terminal                                    │
  │                                                     │
  │  LED WIRES (2 colored wires):                       │
  │                                                     │
  │  3. Connect one colored wire to power supply (+)    │
  │                                                     │
  │  4. Connect the other colored wire to power         │
  │     supply (−)  (polarity does not matter)          │
  │                                                     │
  │  Button wiring diagram:                             │
  │                                                     │
  │     Button                 Relay Controller         │
  │     ┌──────────┐           ┌─────────────┐          │
  │     │ Black 1  │───────────│ INPUT/SIG   │          │
  │     │ Black 2  │───────────│ GND         │          │
  │     └──────────┘           └─────────────┘          │
  │                                                     │
  │     ┌──────────┐           ┌─────────────┐          │
  │     │ Color 1  │───────────│ 12V (+)     │          │
  │     │ Color 2  │───────────│ 12V (−)     │          │
  │     └──────────┘           └─────────────┘          │
  │                                                     │
  └─────────────────────────────────────────────────────┘
```
### Step 2 — Set Up the WiFi Relay Module

```
  ┌─────────────────────────────────────────────────────┐
  │                                                     │
  │  1. Download the Tuya or eWeLink app on your phone  │
  │     (check your relay's box for the correct app)    │
  │                                                     │
  │  2. Follow the app instructions to add the relay    │
  │     to your WiFi network                            │
  │                                                     │
  │  3. In Home Assistant, install the Tuya or eWeLink  │
  │     integration (Settings → Integrations → + Add)   │
  │                                                     │
  │  4. The relay will appear as a "Switch" in HA       │
  │                                                     │
  └─────────────────────────────────────────────────────┘
```
### Step 3 — Power the Relay Module

```
  ┌─────────────────────────────────────────────────────┐
  │                                                     │
  │  BEFORE YOU START: Unplug your power supply!        │
  │                                                     │
  │  ① Take your DC power supply output wires           │
  │    (Red = positive +, Black = negative -)           │
  │                                                     │
  │  ② Use a screwdriver to loosen the VCC terminal     │
  │    screw on the relay module                        │
  │                                                     │
  │  ③ Insert the RED wire into VCC terminal            │
  │                                                     │
  │  ④ Tighten the screw to hold the wire               │
  │                                                     │
  │  ⑤ Use a screwdriver to loosen the GND terminal     │
  │    screw on the relay module                        │
  │                                                     │
  │  ⑥ Insert the BLACK wire into GND terminal          │
  │                                                     │
  │  ⑦ Tighten the screw to hold the wire               │
  │                                                     │
  │     Power Supply          Relay Module              │
  │     ┌──────────┐          ┌─────────────┐           │
  │     │ (+) Red  │──────────│ VCC         │           │
  │     │ (-) Black│──────────│ GND         │           │
  │     └──────────┘          └─────────────┘           │
  │                                                     │
  └─────────────────────────────────────────────────────┘
```

---

### Step 4 — Connect the Pump to the Relay

```
  ┌─────────────────────────────────────────────────────┐
  │                                                     │
  │  BEFORE YOU START: Unplug your pump from the wall!  │
  │                                                     │
  │  Your pump has 3 wires coming out of its power cord:│
  │                                                     │
  │    🔴 LIVE (Brown or Black)   = carries the power   │
  │    🔵 NEUTRAL (Blue or White) = return wire         │
  │    🟢 EARTH (Green/Yellow)    = safety ground       │
  │                                                     │
  │  ① Cut the pump power cord (do not cut near plug!)  │
  │    Leave enough length to reach the relay module    │
  │                                                     │
  │  ② Strip a small amount of insulation from each wire│
  │                                                     │
  │  ③ The LIVE wire from the wall ─── goes into COM    │
  │                                                     │
  │  ④ The LIVE wire to the pump ──── goes into NO      │
  │                                                     │
  │  ⑤ NEUTRAL wires connect directly                  │
  │    (wall neutral ──── pump neutral, no relay)       │
  │                                                     │
  │  ⑥ EARTH wires connect directly                    │
  │    (wall earth ──── pump earth, no relay)           │
  │                                                     │
  │  Relay Module (output side)                         │
  │  ┌────────────────────────────┐                     │
  │  │ COM │ NO  │ NC             │                     │
  │  └──┬──┴──┬──┴────────────────┘                     │
  │     │     │                                         │
  │  LIVE  LIVE                                         │
  │  from  to                                           │
  │  wall  pump                                         │
  │                                                     │
  └─────────────────────────────────────────────────────┘
```

---

### Step 5 — Verify All Connections

```
  ┌─────────────────────────────────────────────────────┐
  │  FINAL CHECKLIST before powering on:               │
  │                                                     │
  │  □  Power supply (+) connected to relay VCC         │
  │  □  Power supply (-) connected to relay GND         │
  │  □  Wall LIVE wire connected to relay COM           │
  │  □  Pump LIVE wire connected to relay NO            │
  │  □  Wall NEUTRAL connected directly to pump NEUTRAL │
  │  □  Wall EARTH connected directly to pump EARTH     │
  │  □  All terminal screws are tight                   │
  │  □  No bare wires are touching each other           │
  │  □  Button black wires connected to relay controller   │
  │  □  Button colored wires connected to 12V power supply │
  │  □  WiFi relay is paired to app and Home Assistant  │
  │                                                     │
  │  ✅ All checked? Safe to power on!                  │
  │                                                     │
  └─────────────────────────────────────────────────────┘
```

---

## Home Assistant Setup

### Step 1 — Add the Hardwired Button as a Binary Sensor

The FLM19-FW-13 is wired directly to your relay controller. Home Assistant sees it as a **binary sensor** (ON when button is held, OFF when released):

1. Go to **Settings** → **Devices & Services**
2. Find your relay controller device and open it
3. Look for the binary sensor entity that corresponds to the button input
4. Note its entity name (e.g., `binary_sensor.ro_pump_button`)

> **Note:** If your relay controller does not expose a binary sensor for the button input, you may need to configure one in your `configuration.yaml`. Example for a Raspberry Pi GPIO pin:
> ```yaml
> binary_sensor:
>   - platform: gpio
>     pin: 17          # ← Replace with the GPIO pin number connected to the button's black wire
>     name: "RO Pump Button"
>     device_class: connectivity
> ```
> See the [Home Assistant GPIO Binary Sensor docs](https://www.home-assistant.io/integrations/binary_sensor.gpio/) for details.

### Step 2 — Create an Input Boolean Helper

This tracks whether the pump is currently running:

1. Go to **Settings** → **Devices & Services** → **Helpers**
2. Click **+ Create Helper**
3. Choose **Toggle**
4. Set:
   - **Name:** `RO Pump Running`
5. Click **Create**

### Step 3 — Find Your Device Names

1. Go to **Settings** → **Devices & Services**
2. Find your **Button Binary Sensor** — note its entity name (e.g., `binary_sensor.ro_pump_button`)
3. Find your **WiFi Relay** — note its entity name (e.g., `switch.ro_pump_relay`)

---
## Automation Code

Copy and paste this into your Home Assistant automations. Replace `binary_sensor.ro_pump_button` with your actual button entity name, and `switch.ro_pump_relay` with your actual relay entity name.

### Automation 1 — Hold-Duration Timer

This automation triggers when the button is released (toggled back to OFF) and checks how long it was held:

```yaml
alias: "RO Pump - Hold Duration Timer"
description: "Hold 2s = 1 min, hold 3s = 3 min, hold 5s = 5 min"
trigger:
  - platform: state
    entity_id: binary_sensor.ro_pump_button   # ← Replace with your button entity
    to: "off"
condition:
  - condition: state
    entity_id: input_boolean.ro_pump_running
    state: "off"                              # Only start if pump is not already running
action:
  - variables:
      hold_seconds: >
        {% if trigger.from_state.state == "on" %}
          {{ (now() - trigger.from_state.last_changed).total_seconds() | int }}
        {% else %}
          0
        {% endif %}
  - choose:
      - conditions:
          - condition: template
            value_template: "{{ hold_seconds >= 5 }}"
        sequence:
          - service: input_boolean.turn_on
            target:
              entity_id: input_boolean.ro_pump_running
          - service: switch.turn_on
            target:
              entity_id: switch.ro_pump_relay  # ← Replace with your relay entity
          - delay:
              minutes: 5                        # 5 MINUTES timer
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay
          - service: input_boolean.turn_off
            target:
              entity_id: input_boolean.ro_pump_running
      - conditions:
          - condition: template
            value_template: "{{ hold_seconds >= 3 }}"
        sequence:
          - service: input_boolean.turn_on
            target:
              entity_id: input_boolean.ro_pump_running
          - service: switch.turn_on
            target:
              entity_id: switch.ro_pump_relay
          - delay:
              minutes: 3                        # 3 MINUTES timer
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay
          - service: input_boolean.turn_off
            target:
              entity_id: input_boolean.ro_pump_running
      - conditions:
          - condition: template
            value_template: "{{ hold_seconds >= 2 }}"
        sequence:
          - service: input_boolean.turn_on
            target:
              entity_id: input_boolean.ro_pump_running
          - service: switch.turn_on
            target:
              entity_id: switch.ro_pump_relay
          - delay:
              minutes: 1                        # 1 MINUTE timer
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay
          - service: input_boolean.turn_off
            target:
              entity_id: input_boolean.ro_pump_running
      # If held less than 2 seconds, do nothing (accidental press ignored)
mode: single
```

> **How it works:**
> - `trigger.from_state.last_changed` is the timestamp when the button state became "on" (when you pressed it)
> - When the button goes back to "off" (released), Home Assistant calculates the duration
> - The `choose` block checks longest duration first (5s → 3s → 2s) so it always matches the right tier
> - Holds shorter than 2 seconds are silently ignored (prevents accidental triggers)

### Automation 2 — Emergency Stop (Safety!)

```yaml
alias: "RO Pump - Emergency Stop"
description: "Toggle the button ON then OFF quickly to stop the pump immediately"
trigger:
  - platform: state
    entity_id: binary_sensor.ro_pump_button   # ← Replace with your button entity
    to: "off"
condition:
  - condition: state
    entity_id: input_boolean.ro_pump_running
    state: "on"                               # Only stop if pump is actually running
  - condition: template
    value_template: >
      {{ trigger.from_state.state == "on" and
         (now() - trigger.from_state.last_changed).total_seconds() < 2 }}
                                              # Quick toggle (< 2 sec) = emergency stop
action:
  - service: switch.turn_off
    target:
      entity_id: switch.ro_pump_relay         # ← Replace with your relay entity
  - service: input_boolean.turn_off
    target:
      entity_id: input_boolean.ro_pump_running
mode: single
```

---
## Testing the System

```
  ┌─────────────────────────────────────────────────────┐
  │  TEST 1: Button connects to Home Assistant          │
  │                                                     │
  │  1. Go to HA → Settings → Devices                   │
  │  2. Find your button binary sensor entity           │
  │  3. Press and hold the button                       │
  │  4. Watch the entity state change to "on"           │
  │  5. Release the button — state should go to "off"   │
  │  6. ✅ If it toggles, the button is working         │
  └─────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────┐
  │  TEST 2: Relay switches on and off                  │
  │                                                     │
  │  1. Go to HA → Settings → Devices                   │
  │  2. Find your relay device                          │
  │  3. Click the toggle to turn it ON                  │
  │  4. Watch the relay click and LED light up          │
  │  5. Click the toggle to turn it OFF                 │
  │  6. Watch the relay click and LED turn off          │
  │  7. ✅ If it clicks, the relay is working           │
  └─────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────┐
  │  TEST 3: Full System Test (with water disconnected) │
  │                                                     │
  │  1. Enable the automation                           │
  │  2. Hold button ~2 sec, release → 1 minute pump     │
  │  3. Hold button ~3 sec, release → 3 minutes pump    │
  │  4. Hold button ~5 sec, release → 5 minutes pump    │
  │  5. ✅ If all three work, your system is ready!     │
  └─────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────┐
  │  TEST 4: Emergency Stop Test                        │
  │                                                     │
  │  1. Start a pump timer (hold 2+ sec)                │
  │  2. While pump is running, quickly toggle the       │
  │     button ON and back OFF (< 2 seconds)            │
  │  3. The pump should stop immediately                │
  │  4. ✅ Emergency stop is working                    │
  └─────────────────────────────────────────────────────┘
```

---
## Troubleshooting

| Problem | Likely Cause | Solution |
|---------|-------------|---------|
| Button hold does nothing | Button not linked to HA as binary sensor | Check that the button's black wires are connected to the relay controller input; verify the `binary_sensor` entity appears in HA |
| Button LED does not light up | LED wires not connected | Check the colored wires are connected to the 12V power supply (non-polarized — swap if needed) |
| Pump runs but won't stop | Automation error | Check HA logs → Settings → System → Logs |
| Relay clicks but pump doesn't start | Wiring issue | Check COM and NO are connected, not COM and NC |
| Pump runs continuously | Wired to NC instead of NO | Move pump wire from NC terminal to NO terminal |
| Hold duration triggers wrong timer | Hold too short or too long | Practice holding exactly 2, 3, or 5 seconds; watch HA logs to see `hold_seconds` value |
| Accidental 2-second hold starts pump | Threshold too low | Increase the `>= 2` threshold in the automation template (e.g., `>= 3`) so shorter holds are ignored |
| WiFi relay drops offline | Weak WiFi signal | Move router/extender closer or use a WiFi extender |
| Pump starts but `ro_pump_running` is wrong | Multiple triggers | Ensure automation mode is `single` and `input_boolean.ro_pump_running` is toggled correctly |

---
## Quick Reference Card

Print this out and keep it near your RO system:

```
  ╔══════════════════════════════════════════╗
  ║        RO PUMP QUICK REFERENCE           ║
  ╠══════════════════════════════════════════╣
  ║  Hold 2 sec = 1 minute  (quick rinse)    ║
  ║  Hold 3 sec = 3 minutes (normal fill)    ║
  ║  Hold 5 sec = 5 minutes (full fill)      ║
  ║  Quick toggle (< 2 sec) = EMERGENCY STOP ║
  ╚══════════════════════════════════════════╝
```

---

## Conclusion

Your one-button RO pump hold-duration timer is now fully set up! Here is a recap of what you built:

- ✅ **FLM19-FW-13 Hardwired Button** — IP67 waterproof, latching toggle, 4-wire (2 black = switch, 2 colored = LED)
- ✅ **Home Assistant** — Measures how long you hold the button and manages the timer
- ✅ **DieseRC WiFi Relay** — Switches the pump power on and off safely
- ✅ **Hold-duration timer** — Hold 2 sec = 1 min, hold 3 sec = 3 min, hold 5 sec = 5 min
- ✅ **Emergency stop** — Quick toggle (< 2 sec) stops the pump immediately

If you need help or something isn't working, check the [Home Assistant Community Forums](https://community.home-assistant.io/) — there are thousands of helpful people there!

Happy automating! 🎉
