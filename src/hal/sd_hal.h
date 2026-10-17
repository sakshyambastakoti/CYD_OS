/**
 * @file sd_hal.h
 * @brief MicroSD Card Hardware Abstraction Layer for CYD OS
 * 
 * Handles SD card initialization on VSPI bus, mount/unmount,
 * status checking, and basic file operations.
 */

#pragma once

#include <stdint.h>
#include <SD.h>

namespace CydOS {
namespace HAL {

/**
 * @brief Mount result codes
 */
enum class SDStatus {
    OK,              // Card mounted and accessible
    NO_CARD,         // No card detected
    MOUNT_FAILED,    // Card detected but mount failed
    NOT_INITIALIZED  // init() not yet called
};

/**
 * @brief Initialize the SD card on VSPI bus
 * @return SDStatus indicating result
 */
SDStatus sd_init();

/**
 * @brief Unmount the SD card safely
 */
void sd_unmount();

/**
 * @brief Get current SD card status
 */
SDStatus sd_get_status();

/**
 * @brief Check if SD card is currently mounted
 */
bool sd_is_mounted();

/**
 * @brief Get SD card total size in bytes
 * @return Total size, or 0 if not mounted
 */
uint64_t sd_get_total_size();

/**
 * @brief Get SD card used size in bytes
 * @return Used size, or 0 if not mounted
 */
uint64_t sd_get_used_size();

/**
 * @brief Get SD card type as string
 * @return "SD", "SDHC", "SDXC", or "Unknown"
 */
const char* sd_get_type_string();

/**
 * @brief Test SD card read/write capability
 * @return true if a test file was successfully written and read back
 */
bool sd_test_readwrite();

} // namespace HAL
} // namespace CydOS
