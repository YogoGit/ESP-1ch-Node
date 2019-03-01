ESP-1ch-Node
============
As purchased from [here](https://www.banggood.com/2Pcs-Wemos-TTGO-LORA32-868915Mhz-ESP32-LoRa-OLED-0_96-Inch-Blue-Display-p-1239769.html).

## Install [PlatformIO](https://platformio.org/) (command line)

https://docs.platformio.org/en/latest/installation.html#installer-script

## Start
- This is the 'node' code (as compared to the gateway code), as put
  together from numerous projects.

## Build
  To build is pretty easy (and fast), just run:
   ```
   % platformio run
   Processing ttgo-lora32-v1 (platform: espressif32; board: ttgo-lora32-v1; framework: arduino)
   ...
   Building .pioenvs/ttgo-lora32-v1/firmware.bin
   Checking size .pioenvs/ttgo-lora32-v1/firmware.elf
   Memory Usage -> http://bit.ly/pio-memory-usage
   DATA:    [          ]   4.5% (used 14900 bytes from 327680 bytes)
   PROGRAM: [==        ]  16.2% (used 212516 bytes from 1310720 bytes)
   ========================= [SUCCESS] Took 4.01 seconds =========================
   ```

## Upload
  Similar to build, just with an additional argument
   ```
   % platformio run -t upload
   ```
 
## Notes
You'll need at least two boards, one for the [gateway](https://github.com/YogoGit/ESP-1ch-Gateway-v5.0) and this code for each node.  In a perfect world, the
nodes would read some sensor (temperature/humidity) and send it.

## Case
I designed a case based on the designs of similar cases on Thingiverse, which
includes a spot for the antenna.  It's a tight fit, but it should work fine.

[3D printable case](https://www.thingiverse.com/thing:3443245)

## References
- https://github.com/mcci-catena/arduino-lmic
- https://github.com/cyberman54/ESP32-Paxcounter
- https://github.com/sparkfun/ESP32_LoRa_1Ch_Gateway/tree/master/Firmware/ESP-1CH-TTN-Device-ABP
- https://github.com/vpcola/ESP32SingleChannelGateway
