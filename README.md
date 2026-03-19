# Home Assistant Repository — RO Pump Automation

> **New here? Start with the Setup Guide below — it has everything you need, including all wiring diagrams.**

---

## ⚡ Just Want the Code?

**→ [AUTOMATION-CODE.md](AUTOMATION-CODE.md) — copy-paste ready YAML, nothing else**

Two things to change: your button entity name and your relay entity name.  
That's it. The file tells you exactly where.

---

## 📂 All Documents & Where to Find Them

| File | What's Inside |
|------|--------------|
| **[AUTOMATION-CODE.md](AUTOMATION-CODE.md)** | ⚡ Just the code — copy-paste ready YAML for both automations |
| **[RO-PUMP-AUTOMATION-SETUP.md](RO-PUMP-AUTOMATION-SETUP.md)** | ✅ Main guide — wiring diagrams, setup steps, automation code, troubleshooting |
| **[RO-PUMP-AUTOMATION-GUIDE.md](RO-PUMP-AUTOMATION-GUIDE.md)** | Overview guide — introduction, basic configuration reference |

---

## 🗺️ Wiring Diagrams & Visual Guides (Quick Links)

All diagrams are inside **[RO-PUMP-AUTOMATION-SETUP.md](RO-PUMP-AUTOMATION-SETUP.md)**. Click any link below to jump straight to it:

| Diagram / Visual | Direct Link |
|-----------------|-------------|
| How the one-button cycling timer works | [How It Works](RO-PUMP-AUTOMATION-SETUP.md#how-it-works) |
| Full system overview (button → HA → relay → pump) | [System Overview Diagram](RO-PUMP-AUTOMATION-SETUP.md#system-overview-diagram) |
| IP67 WiFi Push Button diagram | [Component Diagrams](RO-PUMP-AUTOMATION-SETUP.md#component-diagrams) |
| DieseRC Relay Module terminal map (VCC / GND / COM / NO / NC) | [DieseRC Relay Module](RO-PUMP-AUTOMATION-SETUP.md#-dieserc-1-channel-wifi-relay-module-top-view) |
| Power supply wiring | [Power Supply Connections](RO-PUMP-AUTOMATION-SETUP.md#-power-supply-connections) |
| Complete wiring diagram (all connections) | [Complete Wiring Diagram](RO-PUMP-AUTOMATION-SETUP.md#complete-wiring-diagram) |
| Simplified wiring (easy-to-read) | [Simplified Wiring](RO-PUMP-AUTOMATION-SETUP.md#simplified-wiring-easy-to-read-version) |
| Relay "Normally Open" explained | [Relay NO Explained](RO-PUMP-AUTOMATION-SETUP.md#relay-normally-open-explained) |
| One-button cycling timer flow diagram | [Timer Flow Diagram](RO-PUMP-AUTOMATION-SETUP.md#one-button-cycling-timer-flow) |
| Step-by-step wiring instructions | [Step-by-Step Wiring](RO-PUMP-AUTOMATION-SETUP.md#step-by-step-wiring-instructions) |

---

## 🚀 Quick Start

1. **Read the Setup Guide** → [RO-PUMP-AUTOMATION-SETUP.md](RO-PUMP-AUTOMATION-SETUP.md)
2. **Buy the parts** → [Shopping List](RO-PUMP-AUTOMATION-SETUP.md#shopping-list)
3. **Wire everything up** → [Wiring Diagrams](RO-PUMP-AUTOMATION-SETUP.md#wiring-diagrams)
4. **Set up Home Assistant** → [Home Assistant Setup](RO-PUMP-AUTOMATION-SETUP.md#home-assistant-setup)
5. **Copy the automation code** → [AUTOMATION-CODE.md](AUTOMATION-CODE.md) *(just the code, nothing else)*
6. **Test it** → [Testing the System](RO-PUMP-AUTOMATION-SETUP.md#testing-the-system)
7. **Something not working?** → [Troubleshooting](RO-PUMP-AUTOMATION-SETUP.md#troubleshooting)

---

## 🔧 What the System Does

```
Press button 1×  →  Pump runs for  1 minute
Press button 2×  →  Pump runs for  3 minutes
Press button 3×  →  Pump runs for  5 minutes
Hold button 3 s  →  EMERGENCY STOP (pump turns off immediately)
```

One waterproof WiFi button controls everything — no app needed once it's set up.

---

## 📥 How to Download Any Document

1. Click the file link above (e.g., [RO-PUMP-AUTOMATION-SETUP.md](RO-PUMP-AUTOMATION-SETUP.md))
2. Click the **Raw** button (top-right of the file view)
3. Press **Ctrl+S** (Windows) or **Cmd+S** (Mac) to save the file
4. Open it with any text editor or Notepad
