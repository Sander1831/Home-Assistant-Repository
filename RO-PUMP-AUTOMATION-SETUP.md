# RO Pump Automation Setup — One-Button Cycling Timer

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

Each time you press the button, the pump runs for a different duration. It cycles through three timers and then repeats:

```
╔══════════════════════════════════════════════════════╗
║           ONE-BUTTON CYCLING TIMER                   ║
╠══════════════════════════════════════════════════════╣
║  Press 1x  ──►  Pump ON  ──►  1 minute  ──►  Pump OFF ║
║  Press 2x  ──►  Pump ON  ──►  3 minutes ──►  Pump OFF ║
║  Press 3x  ──►  Pump ON  ──►  5 minutes ──►  Pump OFF ║
║  Press 4x  ──►  Cycles back to 1 minute               ║
╚══════════════════════════════════════════════════════╝
```

**The sequence never stops — press the button as many times as you want and it always knows where it is in the cycle.**

---

## Shopping List

| # | Item | Notes |
|---|------|-------|
| 1 | **IP67 Wireless WiFi Push Button** | Waterproof button (e.g., Tuya WiFi smart button) |
| 2 | **DieseRC 1-Channel WiFi Relay Module** | The relay that switches the pump on/off |
| 3 | **RO Pump (your existing pump)** | Standard reverse osmosis pump |
| 4 | **5V or 12V DC Power Supply** | To power the relay module (check relay label) |
| 5 | **Home Assistant device** | Raspberry Pi, NUC, or any computer running HA |
| 6 | **Electrical wire** | 18 AWG for pump wiring |
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
│  ┌──────────────┐         ┌──────────────────────────────┐  │
│  │  IP67 WiFi   │  WiFi   │                              │  │
│  │  Push Button │────────►│     HOME ASSISTANT           │  │
│  │  (wireless)  │  signal │     (your smart home hub)   │  │
│  └──────────────┘         │                              │  │
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
```

**In plain English:**
1. You press the button → Button sends a WiFi signal to Home Assistant
2. Home Assistant counts your presses and picks the right timer (1, 3, or 5 min)
3. Home Assistant tells the WiFi relay to turn ON
4. The relay switches the power to your pump ON
5. When the timer runs out, Home Assistant tells the relay to turn OFF
6. Pump turns OFF!

---

## Component Diagrams

### 📦 IP67 WiFi Push Button (Front View)

```
         ┌─────────────────┐
         │  ┌───────────┐  │
         │  │           │  │
         │  │  PRESS    │  │   ← Big round button (press this!)
         │  │   HERE    │  │
         │  │           │  │
         │  └───────────┘  │
         │                 │
         │  [LED light]    │   ← Blinks when connected to WiFi
         │                 │
         │  ~~~~ IP67 ~~~~ │   ← Waterproof rating
         └─────────────────┘

         No wires needed!
         Runs on battery.
         Connects via WiFi.
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
                    ╔══════════════════════════════════════╗
                    ║     POWER SUPPLY (5V or 12V DC)      ║
                    ║                                      ║
                    ║  [+] Red ────────────────────┐       ║
                    ║  [-] Black ──────────────┐   │       ║
                    ╚══════════════════════════╪═══╪═══════╝
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


STEP 3: WiFi Button (no wires needed!)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Just pair it with your WiFi network using the app — it runs on batteries!


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

### One-Button Cycling Timer Flow

```
  ┌─────────────────────────────────────────────────────────────┐
  │                  BUTTON PRESS FLOW                          │
  └─────────────────────────────────────────────────────────────┘

  You press button
       │
       ▼
  ┌────────────────────────────────────────────────────────────┐
  │  Home Assistant counts presses                             │
  │                                                            │
  │  Counter = 1? ─── YES ──► Set timer: 1 MINUTE             │
  │       │                         │                          │
  │       NO                        │                          │
  │       │                         │                          │
  │  Counter = 2? ─── YES ──► Set timer: 3 MINUTES            │
  │       │                         │                          │
  │       NO                        │                          │
  │       │                         │                          │
  │  Counter = 3? ─── YES ──► Set timer: 5 MINUTES            │
  │       │                         │                          │
  │       NO                        │                          │
  │       │                         │                          │
  │  Reset to 1  ◄──────────────────┘                         │
  └────────────────────────────────────────────────────────────┘
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

### Step 1 — Set Up the WiFi Button

```
  ┌─────────────────────────────────────────────────────┐
  │                                                     │
  │  1. Insert batteries into the IP67 WiFi button      │
  │                                                     │
  │  2. Download the Tuya or eWeLink app on your phone  │
  │     (check your button's box for the correct app)   │
  │                                                     │
  │  3. Follow the app instructions to add the button   │
  │     to your WiFi network                            │
  │                                                     │
  │  4. In Home Assistant, install the Tuya or eWeLink  │
  │     integration (Settings → Integrations → + Add)   │
  │                                                     │
  │  5. The button will appear as a device in HA        │
  │                                                     │
  └─────────────────────────────────────────────────────┘
```

---

### Step 2 — Set Up the WiFi Relay Module

```
  ┌─────────────────────────────────────────────────────┐
  │                                                     │
  │  1. Download the Tuya or eWeLink app on your phone  │
  │     (same app as the button, usually)               │
  │                                                     │
  │  2. Follow the app instructions to add the relay    │
  │     to your WiFi network                            │
  │                                                     │
  │  3. In Home Assistant, it should already appear     │
  │     if you set up the integration in Step 1         │
  │                                                     │
  │  4. The relay will appear as a "Switch" in HA       │
  │                                                     │
  └─────────────────────────────────────────────────────┘
```

---

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
  │  □  WiFi button is paired to app and Home Assistant │
  │  □  WiFi relay is paired to app and Home Assistant  │
  │                                                     │
  │  ✅ All checked? Safe to power on!                  │
  │                                                     │
  └─────────────────────────────────────────────────────┘
```

---

## Home Assistant Setup

### Step 1 — Create a Counter Helper

The counter tracks how many times you press the button (1, 2, or 3):

1. Go to **Settings** → **Devices & Services** → **Helpers**
2. Click **+ Create Helper**
3. Choose **Counter**
4. Set:
   - **Name:** `RO Pump Press Counter`
   - **Minimum:** `0`
   - **Maximum:** `3`
   - **Step:** `1`
5. Click **Create**

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
2. Find your **WiFi Button** — note its entity name (e.g., `binary_sensor.wifi_button`)
3. Find your **WiFi Relay** — note its entity name (e.g., `switch.ro_pump_relay`)

---

## Automation Code

Copy and paste this into your Home Assistant automations. Replace `binary_sensor.wifi_button` with your actual button entity name, and `switch.ro_pump_relay` with your actual relay entity name.

### Automation 1 — Count Button Presses

```yaml
alias: "RO Pump - Count Button Press"
description: "Counts how many times the button is pressed and picks the timer"
trigger:
  - platform: state
    entity_id: binary_sensor.wifi_button   # ← Replace with your button entity
    to: "on"
condition:
  - condition: state
    entity_id: input_boolean.ro_pump_running
    state: "off"                           # Only count if pump is not running
action:
  - service: counter.increment
    target:
      entity_id: counter.ro_pump_press_counter
  - delay:
      seconds: 4                           # Wait 4 seconds for more presses
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
              entity_id: switch.ro_pump_relay  # ← Replace with your relay entity
          - delay:
              minutes: 1                   # 1 MINUTE timer
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay
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
              entity_id: switch.ro_pump_relay
          - delay:
              minutes: 3                   # 3 MINUTES timer
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay
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
              entity_id: switch.ro_pump_relay
          - delay:
              minutes: 5                   # 5 MINUTES timer
          - service: switch.turn_off
            target:
              entity_id: switch.ro_pump_relay
          - service: input_boolean.turn_off
            target:
              entity_id: input_boolean.ro_pump_running
          - service: counter.reset
            target:
              entity_id: counter.ro_pump_press_counter
mode: single
```

### Automation 2 — Emergency Stop (Safety!)

```yaml
alias: "RO Pump - Emergency Stop"
description: "Long press the button to stop the pump immediately"
trigger:
  - platform: state
    entity_id: binary_sensor.wifi_button   # ← Replace with your button entity
    to: "on"
    for:
      seconds: 3                           # Hold button for 3 seconds = STOP
condition: []
action:
  - service: switch.turn_off
    target:
      entity_id: switch.ro_pump_relay      # ← Replace with your relay entity
  - service: input_boolean.turn_off
    target:
      entity_id: input_boolean.ro_pump_running
  - service: counter.reset
    target:
      entity_id: counter.ro_pump_press_counter
mode: single
```

---

## Testing the System

```
  ┌─────────────────────────────────────────────────────┐
  │  TEST 1: Button connects to Home Assistant          │
  │                                                     │
  │  1. Go to HA → Settings → Devices                   │
  │  2. Find your button device                         │
  │  3. Press the button                                │
  │  4. Watch the entity state change to "on"           │
  │  5. ✅ If it changes, the button is working         │
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
  │  2. Press button ONCE → pump should run 1 minute    │
  │  3. Press button TWICE → pump should run 3 minutes  │
  │  4. Press button THREE TIMES → pump should run 5min │
  │  5. ✅ If all three work, your system is ready!     │
  └─────────────────────────────────────────────────────┘
```

---

## Troubleshooting

| Problem | Likely Cause | Solution |
|---------|-------------|---------|
| Button press does nothing | Button not linked to HA | Check Tuya/eWeLink integration in HA Settings → Integrations |
| Pump runs but won't stop | Automation error | Check HA logs → Settings → System → Logs |
| Relay clicks but pump doesn't start | Wiring issue | Check COM and NO are connected, not COM and NC |
| Pump runs continuously | Wired to NC instead of NO | Move pump wire from NC terminal to NO terminal |
| Counter doesn't reset | Automation mode issue | Change automation mode to `single` |
| Second/third press doesn't work | Press too slow | Press all buttons within 4 seconds (the collection window) |
| WiFi relay drops offline | Weak WiFi signal | Move router/extender closer or use a WiFi extender |
| Pump starts but counter is wrong | Multiple triggers | Add condition to check `input_boolean.ro_pump_running` is off |

---

## Quick Reference Card

Print this out and keep it near your RO system:

```
  ╔══════════════════════════════════════╗
  ║      RO PUMP QUICK REFERENCE         ║
  ╠══════════════════════════════════════╣
  ║  Press 1x = 1 minute (quick rinse)   ║
  ║  Press 2x = 3 minutes (normal fill)  ║
  ║  Press 3x = 5 minutes (full fill)    ║
  ║  Hold 3s  = EMERGENCY STOP           ║
  ╚══════════════════════════════════════╝
```

---

## Conclusion

Your one-button RO pump cycling timer is now fully set up! Here is a recap of what you built:

- ✅ **IP67 WiFi Button** — Wireless, waterproof, battery-powered
- ✅ **Home Assistant** — Counts your button presses and manages the timer
- ✅ **DieseRC WiFi Relay** — Switches the pump power on and off safely
- ✅ **One-button cycling** — 1 press = 1 min, 2 presses = 3 min, 3 presses = 5 min
- ✅ **Emergency stop** — Hold button 3 seconds to stop pump immediately

If you need help or something isn't working, check the [Home Assistant Community Forums](https://community.home-assistant.io/) — there are thousands of helpful people there!

Happy automating! 🎉
