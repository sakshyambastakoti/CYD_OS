/**
 * @file sd_hal.cpp
 * @brief MicroSD Card HAL implementation for CYD OS
 * 
 * Uses the dedicated VSPI bus (GPIO 18/19/23/5) to avoid
 * conflicts with the display HSPI bus.
 */

#include "sd_hal.h"
#include "pin_config.h"
#include <SPI.h>
#include <SD.h>

// ==========================================================================
// Module state
// ==========================================================================
static SPIClass s_sd_spi(VSPI);
static bool     s_mounted = false;
static CydOS::HAL::SDStatus s_status = CydOS::HAL::SDStatus::NOT_INITIALIZED;

namespace CydOS {
namespace HAL {

SDStatus sd_init() {
    Serial.println("[SD] Initializing SD card on VSPI bus...");

    // Initialize VSPI for SD card
    s_sd_spi.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

    // Attempt mount
    if (!SD.begin(PIN_SD_CS, s_sd_spi)) {
        Serial.println("[SD] ERROR: SD card mount failed!");
        s_mounted = false;

        // Try to determine if it's no card vs mount failure
        // SD.begin returns false for both cases on ESP32
        s_status = SDStatus::MOUNT_FAILED;
        return s_status;
    }

    // Check card type
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[SD] No SD card detected");
        s_mounted = false;
        s_status = SDStatus::NO_CARD;
        return s_status;
    }

    s_mounted = true;
    s_status = SDStatus::OK;

    Serial.printf("[SD] Card mounted: Type=%s, Size=%llu MB\n",
                  sd_get_type_string(),
                  sd_get_total_size() / (1024 * 1024));

    return s_status;
}

void sd_unmount() {
    if (s_mounted) {
        SD.end();
        s_mounted = false;
        s_status = SDStatus::NOT_INITIALIZED;
        Serial.println("[SD] Card unmounted");
    }
}

SDStatus sd_get_status() {
    return s_status;
}

bool sd_is_mounted() {
    return s_mounted;
}

uint64_t sd_get_total_size() {
    if (!s_mounted) return 0;
    return SD.totalBytes();
}

uint64_t sd_get_used_size() {
    if (!s_mounted) return 0;
    return SD.usedBytes();
}

const char* sd_get_type_string() {
    if (!s_mounted) return "None";

    switch (SD.cardType()) {
        case CARD_MMC:  return "MMC";
        case CARD_SD:   return "SD";
        case CARD_SDHC: return "SDHC";
        default:        return "Unknown";
    }
}

bool sd_test_readwrite() {
    if (!s_mounted) {
        Serial.println("[SD] Test failed: card not mounted");
        return false;
    }

    const char* testPath = "/.cyd_os_test";
    const char* testData = "CYD_OS_SD_TEST_OK";

    // Write test
    File writeFile = SD.open(testPath, FILE_WRITE);
    if (!writeFile) {
        Serial.println("[SD] Test failed: cannot open file for writing");
        return false;
    }
    size_t written = writeFile.print(testData);
    writeFile.close();

    if (written == 0) {
        Serial.println("[SD] Test failed: write returned 0 bytes");
        return false;
    }

    // Read test
    File readFile = SD.open(testPath, FILE_READ);
    if (!readFile) {
        Serial.println("[SD] Test failed: cannot open file for reading");
        return false;
    }

    char readBuf[32] = {0};
    size_t readLen = readFile.readBytes(readBuf, sizeof(readBuf) - 1);
    readFile.close();

    // Clean up test file
    SD.remove(testPath);

    if (readLen == 0 || strcmp(readBuf, testData) != 0) {
        Serial.printf("[SD] Test failed: read mismatch (got '%s')\n", readBuf);
        return false;
    }

    Serial.println("[SD] Read/write test PASSED");
    return true;
}

} // namespace HAL
} // namespace CydOS
