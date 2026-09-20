/*!
 * @file OpenFIRENunchuk.h
 * @brief Wii Nunchuk support via I2C (Peripherals bus / Wire1) for OpenFIRE.
 *
 * @details Reads the analog stick and Z/C buttons from a Wii Nunchuk and
 *          feeds them into the existing OpenFIRE input pipeline:
 *            - Stick X/Y  -> same path as the analog stick (Gamepad stick / d-pad / keys)
 *            - Z button   -> duplicates Button A
 *            - C button   -> Button C
 *
 *          [ESP32_PORT] Ported from the RP2040 "MANCO" build. On ESP32-S3 the
 *          "Peripherals SDA/SCL" I2C pins (periphSDA/periphSCL) are freely
 *          remappable to any Wire/Wire1 bus via Wire1.setPins() - there is no
 *          RP2040-style I2C0/I2C1 hardware-block selection by GPIO parity, so
 *          that part of the original Init() has been rewritten. Everything
 *          else (handshake, polling, debouncing, hot-plug retry) is unchanged.
 *
 *          Wiring (ESP32-S3, default OpenFIRE "Peripherals" I2C pins - see
 *          your board's pin table in boards/OpenFIREshared.h for the exact
 *          GPIO numbers):
 *            Nunchuk SDA  -> "Peripherals SDA" pin
 *            Nunchuk SCL  -> "Peripherals SCL" pin
 *            Nunchuk 3V   -> 3V3 OUT -- NOT VBUS/5V!
 *            Nunchuk GND  -> any GND pin
 *
 * @copyright GNU Lesser General Public License
 */

#ifndef _OPENFIRENUNCHUK_H_
#define _OPENFIRENUNCHUK_H_

#include <stdint.h>
#include <Wire.h>

class OF_Nunchuk
{
public:
    /// @brief  Initializes the I2C bus (based on the Peripherals SDA/SCL pin
    ///         mapping) and sends the unencrypted-mode init handshake to the Nunchuk.
    /// @return true if the Nunchuk responded and looks ready.
    static bool Init();

    /// @brief  Reads the Nunchuk over I2C if the polling interval has elapsed.
    ///         Safe to call every loop iteration - internally throttled.
    static void Poll();

    /// @brief  True if Init() succeeded and the Nunchuk is considered present.
    static inline bool available = false;

    /// @brief  Raw stick position, 0-255, center ~128.
    static inline uint8_t stickX = 128;
    static inline uint8_t stickY = 128;

    /// @brief  Debounced button states - true while held.
    static inline bool zPressed = false;
    static inline bool cPressed = false;

private:
    static inline TwoWire* bus = nullptr;
    static inline uint8_t buf[6] = {0};
    static inline unsigned long lastPollMillis = 0;
    static inline bool zPressedRaw = false;
    static inline bool cPressedRaw = false;
    static inline uint8_t consecutiveFails = 0;
    static inline unsigned long lastRetryMillis = 0;

    // Minimum ms between I2C reads - keep this well above the actual I2C transaction time.
    static constexpr unsigned long pollIntervalMs = 16;

    // How often to retry Init() while the Nunchuk isn't detected (hot-plug support).
    static constexpr unsigned long retryIntervalMs = 1500;

    // Consecutive failed reads before considering the Nunchuk disconnected
    // (allows recovery from an occasional I2C hiccup without false-triggering a retry storm).
    static constexpr uint8_t maxConsecutiveFails = 8;

    static bool ReadRaw();
};

#endif // _OPENFIRENUNCHUK_H_