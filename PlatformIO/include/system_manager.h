/**
 * @file system_manager.h
 * @brief System management and control logic for ESP32-S3 monitoring display
 * 
 * Handles system-level control logic, state management, and coordination
 * between subsystems. Provides clean separation between system rules and
 * UI implementation.
 * 
 * @author ESP32-S3 Display Project
 * @date 2025
 */

#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>

// ============================================================================
// CONSTANTS AND CONFIGURATION
// ============================================================================

#define METER_HIDE_TIMEOUT_MS    60000  ///< Hide meter after 1 minute of zero values
#define DISPLAY_BLANK_TIMEOUT_MS 60000  ///< Blank display after 1 minute of no data

// ============================================================================
// SYSTEM STATE VARIABLES
// ============================================================================

// Data reception tracking
extern unsigned long sys_last_data_received_time;   ///< Timestamp of last valid data reception
extern bool sys_first_data_received;                ///< Flag: true after first valid JSON data

// Meter hiding system state
extern unsigned long sys_cpu_temp_zero_start_time;  ///< Timestamp when CPU temp became 0
extern unsigned long sys_cpu_load_zero_start_time;  ///< Timestamp when CPU load became 0
extern bool sys_cpu_temp_meter_hidden;              ///< Flag: true when temp meter is hidden
extern bool sys_cpu_load_meter_hidden;              ///< Flag: true when load meter is hidden
extern int sys_last_cpu_temp;                       ///< Last received CPU temperature value
extern int sys_last_cpu_load;                       ///< Last received CPU load value

// Display power management state
extern bool sys_display_blanked;                    ///< Flag: true when display is blanked

// ============================================================================
// SYSTEM INITIALIZATION
// ============================================================================

/**
 * @brief Initialize system manager and all subsystems
 */
void system_manager_init();

// ============================================================================
// DATA PROCESSING AND VALIDATION
// ============================================================================

/**
 * @brief Process new system monitoring data
 * @param cpu_temp Current CPU temperature (0-100°C)
 * @param cpu_load Current CPU load percentage (0-100%)
 */
void system_process_data(int cpu_temp, int cpu_load);

/**
 * @brief Handle first valid data reception
 */
void system_handle_first_data();

// ============================================================================
// AUTOMATIC METER HIDING SYSTEM
// ============================================================================

/**
 * @brief Update meter hiding logic based on new data values
 * @param cpu_temp Current CPU temperature (0-100)
 * @param cpu_load Current CPU load percentage (0-100)
 */
void system_update_meter_values(int cpu_temp, int cpu_load);

/**
 * @brief Check if meters should be hidden due to prolonged zero values
 */
void system_check_meter_hiding_conditions();

/**
 * @brief Hide CPU temperature meter
 */
void system_hide_cpu_temp_meter();

/**
 * @brief Show CPU temperature meter
 */
void system_show_cpu_temp_meter();

/**
 * @brief Hide CPU load meter
 */
void system_hide_cpu_load_meter();

/**
 * @brief Show CPU load meter
 */
void system_show_cpu_load_meter();

// ============================================================================
// DISPLAY POWER MANAGEMENT SYSTEM
// ============================================================================

/**
 * @brief Check if display should be blanked due to data timeout
 */
void system_check_data_timeout();

/**
 * @brief Blank entire display to save power
 */
void system_blank_entire_display();

/**
 * @brief Restore display when new data arrives
 */
void system_show_entire_display();

// ============================================================================
// SYSTEM MONITORING AND MAINTENANCE
// ============================================================================

/**
 * @brief Perform periodic system maintenance tasks
 */
void system_periodic_update();

/**
 * @brief Get system status information
 * @return String containing current system status
 */
String system_get_status();

#endif // SYSTEM_MANAGER_H
