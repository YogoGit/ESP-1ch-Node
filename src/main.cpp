#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <pins_arduino.h>
#include "images.h"

// Pin mappings for the LoRA radio/library
const lmic_pinmap lmic_pins = {
    .nss =  LORA_CS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LORA_RST,
    .dio = {LORA_IRQ, /* dio1 */ 33, /* dio2 */ 32},
};

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

// LoRaWAN NwkSKey, network session key
static const PROGMEM u1_t NWKSKEY[16] = {
    0xD6, 0x6D, 0xC7, 0x16,
    0xE7, 0x90, 0xB8, 0x39,
    0x8A, 0x6A, 0xE9, 0xA4,
    0x15, 0x48, 0x21, 0x69
};
// LoRaWAN AppSKey, application session key
static const u1_t PROGMEM APPSKEY[16] = {
    0x29, 0x0E, 0x5C, 0xF7,
    0xA1, 0x41, 0x3C, 0x5A,
    0x8B, 0xF9, 0x12, 0x96,
    0xE3, 0xBA, 0x54, 0x40
};

// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = { 0x26041938 };

static uint8_t mydata[] = "Hello, world!";
static osjob_t sendjob;

// Schedule data trasmission in every this many seconds (might become
// longer due to duty cycle limitations).  we set 10 seconds interval
const unsigned TX_INTERVAL = 10;

SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

// Forward method declaration
void do_send(osjob_t *j);
void showLogo();

void onEvent(ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch (ev) {
    case EV_SCAN_TIMEOUT:
        Serial.println(F("EV_SCAN_TIMEOUT"));
        break;

    case EV_BEACON_FOUND:
        Serial.println(F("EV_BEACON_FOUND"));
        break;

    case EV_BEACON_MISSED:
        Serial.println(F("EV_BEACON_MISSED"));
        break;

    case EV_BEACON_TRACKED:
        Serial.println(F("EV_BEACON_TRACKED"));
        break;

    case EV_JOINING:
        Serial.println(F("EV_JOINING"));
        break;

    case EV_JOINED:
        Serial.println(F("EV_JOINED"));
        break;

    case EV_RFU1:
        Serial.println(F("EV_RFU1"));
        break;

    case EV_JOIN_FAILED:
        Serial.println(F("EV_JOIN_FAILED"));
        break;

    case EV_REJOIN_FAILED:
        Serial.println(F("EV_REJOIN_FAILED"));
        break;

    case EV_TXCOMPLETE:
        // Turn off LED after send is complete
        digitalWrite(LED_BUILTIN, LOW);

        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        if (LMIC.txrxFlags & TXRX_ACK)
            Serial.println(F("Received ack"));
        if (LMIC.dataLen) {
            Serial.println(F("Received "));
            Serial.println(LMIC.dataLen);
            Serial.println(F(" bytes of payload"));
        }
        // Schedule the next transmission
        os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
        break;

    case EV_LOST_TSYNC:
        Serial.println(F("EV_LOST_TSYNC"));
        break;

    case EV_RESET:
        Serial.println(F("EV_RESET"));
        break;

    case EV_RXCOMPLETE:
        // data received in ping slot
        Serial.println(F("EV_RXCOMPLETE"));
        if (LMIC.dataLen) {
            Serial.printf("Received %d bytes\n", LMIC.dataLen);
        }
        break;

    case EV_LINK_DEAD:
        Serial.println(F("EV_LINK_DEAD"));
        break;

    case EV_LINK_ALIVE:
        Serial.println(F("EV_LINK_ALIVE"));
        break;

    case EV_TXSTART:
        Serial.println(F("EV_TXSTART"));
        break;

    default:
        Serial.println(F("Unknown event"));
        break;
    }
}

void do_send(osjob_t *j) {
    static int counter = 0;
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else if (!(LMIC.opmode & OP_TXRXPEND)) {
        // Turn on LED to indicate we're sending data.
        digitalWrite(LED_BUILTIN, HIGH);

        display.clear();
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 0, "Sending packet: ");
        display.drawString(90, 0, String(counter, DEC));
        display.display();
        counter++;

        // Prepare upstream data transmission at the next possible time.
        //
        // - Transmit on port 1 (the first parameter); you can use any
        //   value from 1 to 223 (others are reserved).
        // - Don't request an ack (the last parameter, if not zero,
        //   requests an ack from the network).
        // (Acks consume a lot of network resources; don't ask for an
        // ack unless you really need it.)
        LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println();
    Serial.println("LoraWAN Node...");

    // Configure built-in LED -- will illuminate when sending
    pinMode(LED_BUILTIN, OUTPUT);

    // Configure OLED by setting the OLED Reset HIGH, LOW, and then back HIGH
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(50);
    digitalWrite(OLED_RST, HIGH);
    display.init();
    display.flipScreenVertically();

    // LoRa image
    showLogo();
    delay(2000);

    // Indicate which function this device is running
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    int centerWidth = display.getWidth() / 2;
    int centerHeight = display.getHeight() / 2;
    display.drawString(centerWidth, 0, "LoRa");
    display.drawString(centerWidth, centerHeight, "Node (transmitter)");
    display.display();
    delay(2000);

    // LMIC init (LoRaWAN)
    os_init();

    // Reset the MAC state. Session and pending data transfers will be
    // discarded.
    LMIC_reset();

    // Set static session parameters. Instead of dynamically
    // establishing a session by joining the network, precomputed
    // session parameters are be provided.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession(0x1, DEVADDR, nwkskey, appskey);

    // Disable all 72 channels used by TTN
    for (int c = 0; c < 72; c++) {
        LMIC_disableChannel(c);
    }

    // We'll only enable Channel 16 (905.5Mhz) since we're transmitting
    // on a single-channel
    LMIC_enableChannel(16);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to
    // be ignored by the library)
    LMIC_setDrTxpow(DR_SF7, 14);
    Serial.println(F("LMIC setup done!"));

    // Start job -- Transmit a message on begin
    do_send(&sendjob);
}

void loop() {
    // Make sure LMIC is ran too
    os_runloop_once();
}

void showLogo() {
  uint8_t x_off = (display.getWidth() - logo_width) / 2;
  uint8_t y_off = (display.getHeight() - logo_height) / 2;

  display.clear();
  display.drawXbm(x_off, y_off, logo_width, logo_height, logo_bits);
  display.display();
}
