# Wiring Diagrams

---

## Relay Terminals

```
┌──────────────────────────────────────────────┐
│           DieseRC WiFi RELAY MODULE           │
│                                               │
│   POWER IN           SWITCH TERMINALS         │
│  ┌─────┬─────┐      ┌──────┬──────┬──────┐   │
│  │ VCC │ GND │      │ COM  │  NO  │  NC  │   │
│  └─────┴─────┘      └──────┴──────┴──────┘   │
│    (+)   (-)         always  use   ignore     │
└──────────────────────────────────────────────┘
```

---

## Power Supply → Relay

```
Power Supply (+) ──────► Relay VCC
Power Supply (−) ──────► Relay GND
```

---

## Pump Wiring

```
Wall LIVE ─────────────► Relay COM
Relay NO  ─────────────► Pump LIVE
Wall NEUTRAL ──────────► Pump NEUTRAL   (direct, no relay)
Wall EARTH ────────────► Pump EARTH     (direct, no relay)
```

---

## Full Diagram

```
  POWER SUPPLY (5V or 12V DC)
  ┌──────────────────┐
  │  (+) ────────────┼──────────────────► Relay VCC
  │  (−) ────────────┼──────────────────► Relay GND
  └──────────────────┘

  WALL OUTLET (mains)                    RELAY MODULE
  ┌──────────────────┐                ┌────┬────┬────┐
  │  LIVE ───────────┼───────────────►│COM │ NO │ NC │
  │                  │                └────┴──┬─┴────┘
  │                  │                        │
  │                  │             ┌──────────┘
  │                  │             │
  │                  │          ┌──┴───────────────┐
  │  NEUTRAL ────────┼─────────►│ PUMP NEUTRAL     │
  │  EARTH ──────────┼─────────►│ PUMP EARTH       │
  └──────────────────┘          └──────────────────┘
```

---

## Press → Timer

```
1 press  ──►  1 min
2 presses ──►  3 min
3 presses ──►  5 min
Hold 3s   ──►  STOP
```
