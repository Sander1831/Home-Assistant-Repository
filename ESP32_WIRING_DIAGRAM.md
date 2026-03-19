# ESP32 Wiring Diagram

This document provides detailed ASCII wiring diagrams showing the connections of the ESP32 to a relay, switch, and pump. The diagrams include pin numbers and component details for each connection.

## Wiring Diagram

```
          ESP32 Pin Connections
        +---------------------+
        |                     |
        |                  (5V) ------> +12V (Relay Power)
        |                     |
        |                    (GND) ------> GND (Common Ground)
        |                     |
        |                 Pin GPIO 23 ------> Relay IN1 (Channel 1)
        |                     |
        |                 Pin GPIO 22 ------> Relay IN2 (Channel 2)
        |                     |
        |                 Pin GPIO 21 ------> Pump Control (PWM or GPIO)
        |                     |
        |                 Pin GPIO 19 ------> Switch Input (Digital Signal)
        |                     |
        +---------------------+


## Component Details
- **Relay Module**
  - IN1: Connected to GPIO 23
  - IN2: Connected to GPIO 22
  - VCC: Connect to 5V
  - GND: Connect to GND

- **Pump**
  - Control Pin: Connect to GPIO 21 (via transistor if needed)

- **Switch**
  - Input Pin: Connected to GPIO 19 with pull-down resistor

## Notes
- Ensure the common ground is shared among components to prevent floating ground issues.
- The relay channels can be configured based on your requirements for controlling multiple devices.
- Review the ESP32 datasheet for maximum current ratings when connecting devices directly.
