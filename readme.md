# M5StickC plus ESP32 client for Pokit Pro
Pokit Pro multimeter client on M5StickC plus(ESP-PICO-D4) via BLE, base on Arduino.

## Features
- Display all Pokit Pro's multimeter values, use button A to switch measurement type, with auto range
- Auto range display
- Auto follow Pokit Pro's mode switch(Voltage,Resistance/mA,Amperage)
- Cycle through brightness(button B)
- Display M5 and Pokit's battery voltage
- Low power consumption (~35mA, 3.5hours battery life on M5StickC plus)

## Get started
1. M5StickC plus hardware
2. Download Arduino IDE
3. Install M5StickC plus board and library:
    - https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
4. Compile and upload, that's it!

## Support
### For ESP32
Currently only support M5StickC plus. However, the code, including display drawing part may work on any M5 product. Just need to import corresponding headers and adjust those text arragnement.
Also the BLE part of code should also work on any ESP32. All display-related code is inside display.cpp. You can adapt for your display hardware easily.

### For Pokit
Currently only support Pokit Pro. However, it should works with Pokit meter with only modifiying the BLE Service UUID.

## Roadmap
- Fix the mode switch. Currently missing amp measurement when handle is in middle.
- Compile Arduino core with ESP32's DFS(dynamic frequency scaling) for M5 series.
- Create library for ESP32.