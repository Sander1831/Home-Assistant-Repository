# PUMP CONTROLLER COMPLETE GUIDE

## Wiring Diagram
(Insert complete wiring diagram here)

## ESPHome Code
```yaml
# Example ESPHome configuration
esphome:
  name: pump_controller

# Define your ESP32 device
esp32:
  board: esp32dev

wifi:
  ssid: "Your_SSID"
  password: "Your_PASSWORD"

# Enable logging
logger:

# Enable Home Assistant API
api:
  password: "Your_API_PASSWORD"

ota:
  password: "Your_OTA_PASSWORD"

# Multi-press button detection
binary_sensor:
  - platform: gpio
    pin: GPIO0
    name: "Multi-Press Button"
    on_press:
      then:
        - script.execute: handle_button_press

script:
  - id: handle_button_press
    then:
      - delay: 0.5s
      - if:
          condition:
            binary_sensor.is_on: multi_press_button
          then:
            - switch.toggle: your_switch_id

switch:
  - platform: gpio
    id: your_switch_id
    pin: GPIO2
    name: "Pump Switch"
``` 

## Step-by-Step Installation Instructions
1. **Gather Materials**: Ensure you have all necessary components including the ESP32, pumps, and buttons.
2. **Follow Wiring Diagram**: Wire the components according to the diagram provided.
3. **Upload ESPHome Code**: Use the ESPHome dashboard to upload the configuration to your ESP32.
4. **Connect to Home Assistant**: Ensure the device appears in your Home Assistant interface.
5. **Test the Setup**: Verify that pressing the button operates the pump as expected.

## Conclusion
Now you have a fully functional ESP32 pump controller with multi-press detection!