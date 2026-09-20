/*!
 * @file OpenFIRENunchuk.cpp
 * @brief Wii Nunchuk support via I2C for OpenFIRE. See OpenFIRENunchuk.h.
 *
 * [ESP32_PORT] Ported from the RP2040 "MANCO" build. Only Init()'s bus
 * selection changed - it now mirrors the same ARDUINO_ARCH_ESP32 pattern
 * already used by ExtDisplay::Begin() in OpenFIREdisplay.cpp: on ESP32-S3,
 * Wire1.setPins(SDA, SCL) can remap the bus to any GPIO pair, so there's no
 * need (and no correct way) to replicate the RP2040 bitRead(pin, 1) trick
 * used to pick between the two fixed hardware I2C blocks. Everything else
 * (handshake bytes, polling/debounce/retry logic) is unchanged from the
 * original RP2040 module.
 *
 * @copyright GNU Lesser General Public License
 */

#include <Arduino.h>

#include "OpenFIRENunchuk.h"
#include "OpenFIREprefs.h"

namespace {
    constexpr uint8_t NUNCHUK_ADDR = 0x52;
}

bool OF_Nunchuk::Init()
{
    // Bail out if the Peripherals I2C pins aren't mapped for this board/config.
    if(OF_Prefs::pins[OF_Const::periphSDA] < 0 || OF_Prefs::pins[OF_Const::periphSCL] < 0) {
        available = false;
        return false;
    }

    // [ESP32_PORT] On ESP32-S3, Wire1 can be freely remapped to any GPIO pair -
    // there's no fixed I2C0/I2C1 hardware block tied to specific pins like on
    // RP2040, so we always use Wire1 for the "Peripherals" bus, same as
    // ExtDisplay::Begin() does for the OLED.
    Wire1.setPins(OF_Prefs::pins[OF_Const::periphSDA], OF_Prefs::pins[OF_Const::periphSCL]);
    Wire1.begin();
    bus = &Wire1;

    // Unencrypted init handshake - a well-known trick that disables the
    // (pointless) Nunchuk data encryption so we can read plain data bytes.
    bus->beginTransmission(NUNCHUK_ADDR);
    bus->write(0xF0);
    bus->write(0x55);
    if(bus->endTransmission() != 0) {
        available = false;
        return false;
    }
    delay(1);

    bus->beginTransmission(NUNCHUK_ADDR);
    bus->write(0xFB);
    bus->write(0x00);
    if(bus->endTransmission() != 0) {
        available = false;
        return false;
    }
    delay(1);

    available = ReadRaw();
    consecutiveFails = 0;

    return available;
}

bool OF_Nunchuk::ReadRaw()
{
    if(bus == nullptr)
        return false;

    bus->beginTransmission(NUNCHUK_ADDR);
    bus->write(0x00);
    if(bus->endTransmission() != 0)
        return false;

    // Small settling delay the Nunchuk needs between the "request" write
    // and reading the 6 data bytes back.
    delayMicroseconds(200);

    uint8_t got = bus->requestFrom((uint8_t)NUNCHUK_ADDR, (uint8_t)6);
    if(got < 6)
        return false;

    for(uint8_t i = 0; i < 6; ++i)
        buf[i] = bus->read();

    stickX = buf[0];
    stickY = buf[1];
    // Byte 5: bit 0 = ~Z (0 = pressed), bit 1 = ~C (0 = pressed)
    zPressedRaw = !(buf[5] & 0x01);
    cPressedRaw = !(buf[5] & 0x02);

    return true;
}

void OF_Nunchuk::Poll()
{
    unsigned long m = millis();

    if(!available) {
        // Not detected (or lost) - periodically retry the full init handshake
        // so hot-plugging the Nunchuk in later still works.
        if(m - lastRetryMillis >= retryIntervalMs) {
            lastRetryMillis = m;
            Init();
        }
        return;
    }

    if(m - lastPollMillis < pollIntervalMs)
        return;
    lastPollMillis = m;

    if(!ReadRaw()) {
        // Tolerate the occasional I2C hiccup, but if reads keep failing,
        // treat it as disconnected so the retry logic above kicks back in.
        if(++consecutiveFails >= maxConsecutiveFails) {
            available = false;
            lastRetryMillis = m;
        }
        return;
    }

    consecutiveFails = 0;
    zPressed = zPressedRaw;
    cPressed = cPressedRaw;
}