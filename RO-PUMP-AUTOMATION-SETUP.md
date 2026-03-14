# RO Pump Automation Setup

## Overview  
This guide will help you set up a one-button RO pump timer system that can cycle through different timings: 1 minute, 3 minutes, and 5 minutes. This system is designed for beginners and requires basic understanding of Home Assistant.

## Prerequisites  
- Home Assistant installed and running on your device.  
- A compatible RO pump that can be controlled via smart home automation.  

## Components Needed  
- An RF remote control that is capable of sending signals to the pump.  
- Home Assistant compatible devices and accessories.

## Setup Instructions  
### 1. Connecting the Hardware  
- **Step 1**: Connect the RO pump to your smart home system. Ensure it's powered and properly set up.  
- **Step 2**: If using an RF remote, connect it to a receiver that is compatible with Home Assistant.

### 2. Configure Home Assistant  
- Navigate to your Home Assistant dashboard and open the configuration.yaml file.

### 3. Automating the Timer  
You'll want to set up an automation in Home Assistant that can cycle through the timer settings. Here is an example of how you might do this:

```yaml  
automation:
  - alias: "RO Pump Timer"
    trigger:
      platform: state  
      entity_id: input_boolean.ro_pump_button  
      to: 'on'
    action:
      - service: switch.turn_on  
        entity_id: switch.ro_pump  
      - delay: '00:01:00' # 1 Minute  
      - service: switch.turn_off  
        entity_id: switch.ro_pump  
      - delay: '00:01:00' # 1 Minute Pause  
      - service: switch.turn_on  
        entity_id: switch.ro_pump  
      - delay: '00:03:00' # 3 Minutes  
      - service: switch.turn_off  
        entity_id: switch.ro_pump  
      - delay: '00:01:00' # 1 Minute Pause  
      - service: switch.turn_on  
        entity_id: switch.ro_pump  
      - delay: '00:05:00' # 5 Minutes  
      - service: switch.turn_off  
        entity_id: switch.ro_pump  
```

### 4. Testing the System  
- After configuring the automation, go back to your Home Assistant dashboard.  
- Toggle the button for the RO pump to start the cycle.

### 5. Troubleshooting  
- Ensure that all components are properly connected.  
- Check the Home Assistant logs for any errors if the timer does not cycle as expected.

## Conclusion  
Your one-button RO pump timer system is now ready! You can start using it and make adjustments based on your needs. Happy automating!  
