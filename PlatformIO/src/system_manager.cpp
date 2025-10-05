/**
 * @file system_manager.cpp
 * @brief Implementation of system management and control logic
 * 
 * This file implements all system-level control logic, state management, and
 * coordination between different subsystems. It provides a clean separation
 * between system rules and UI implementation.
 * 
 * @author ESP32-S3 Display Project
 * @date 2025
 */

#include "system_manager.h"
#include "ui_components.h"
#include "display_driver.h"
#include <Arduino.h>

// ============================================================================
// SYSTEM STATE VARIABLES
// ============================================================================

// Data reception tracking
unsigned long sys_last_data_received_time = 0;   ///< Timestamp of last valid data reception
bool sys_first_data_received = false;            ///< Flag: true after first valid JSON data

// Meter hiding system state
unsigned long sys_cpu_temp_zero_start_time = 0;  ///< Timestamp when CPU temp became 0
unsigned long sys_cpu_load_zero_start_time = 0;  ///< Timestamp when CPU load became 0
bool sys_cpu_temp_meter_hidden = false;          ///< Flag: true when temp meter is hidden
bool sys_cpu_load_meter_hidden = false;          ///< Flag: true when load meter is hidden
int sys_last_cpu_temp = -1;                      ///< Last received CPU temp (-1 = uninitialized)
int sys_last_cpu_load = -1;                      ///< Last received CPU load (-1 = uninitialized)

// Display power management state
bool sys_display_blanked = false;                ///< Flag: true when display is blanked

// ============================================================================
// SYSTEM INITIALIZATION
// ============================================================================

/**
 * @brief Initialize system manager and all subsystems
 * 
 * Resets all state variables to their initial values and prepares the system
 * for operation. This ensures a clean startup state regardless of any previous
 * system state.
 */
void system_manager_init() {
    // Reset data reception tracking
    sys_last_data_received_time = 0;
    sys_first_data_received = false;
    
    // Reset meter hiding system state
    sys_cpu_temp_zero_start_time = 0;
    sys_cpu_load_zero_start_time = 0;
    sys_cpu_temp_meter_hidden = false;
    sys_cpu_load_meter_hidden = false;
    sys_last_cpu_temp = -1;  // -1 indicates uninitialized
    sys_last_cpu_load = -1;  // -1 indicates uninitialized
    
    // Reset display power management state
    sys_display_blanked = false;
    
    Serial.println("System Manager initialized - ready for data processing");
}

// ============================================================================
// DATA PROCESSING AND VALIDATION
// ============================================================================

/**
 * @brief Process new system monitoring data
 * 
 * This is the main entry point for all system data processing. It coordinates
 * all subsystems and ensures proper state management across the entire system.
 */
void system_process_data(int cpu_temp, int cpu_load) {
    // Update data reception timestamp for timeout monitoring
    sys_last_data_received_time = millis();
    
    // Handle first valid data reception - transition from boot to main UI
    if (!sys_first_data_received) {
        system_handle_first_data();
    }
    
    // Restore display if it was blanked due to data timeout
    if (sys_display_blanked) {
        system_show_entire_display();
    }
    
    // Update automatic meter hiding system with new values
    system_update_meter_values(cpu_temp, cpu_load);
}

/**
 * @brief Handle first valid data reception
 * 
 * Manages the critical transition from boot animation to the main monitoring
 * interface. This only happens once during the system lifecycle.
 */
void system_handle_first_data() {
    sys_first_data_received = true;
    hide_boot_animation();  // Call UI function to hide boot animation
    Serial.println("First JSON data received - switching to main UI");
}

// ============================================================================
// AUTOMATIC METER HIDING SYSTEM
// ============================================================================

/**
 * @brief Update meter hiding logic based on new data values
 * 
 * Implements the automatic meter hiding algorithm that automatically hides
 * meters showing zero values for extended periods, improving the user experience
 * by reducing visual clutter when certain metrics are not relevant.
 */
void system_update_meter_values(int cpu_temp, int cpu_load) {
    unsigned long current_time = millis();
    
    // ========================================================================
    // CPU TEMPERATURE METER LOGIC
    // ========================================================================
    
    if (cpu_temp == 0) {
        // Temperature value is zero
        if (sys_last_cpu_temp != 0) {
            // Just became zero, start the hiding timer
            sys_cpu_temp_zero_start_time = current_time;
            Serial.println("CPU temperature became zero - starting timer");
        }
        // Check if it's been zero for more than the timeout period
        if (current_time - sys_cpu_temp_zero_start_time >= METER_HIDE_TIMEOUT_MS) {
            system_hide_cpu_temp_meter();
        }
    } else {
        // Temperature value is non-zero
        if (sys_cpu_temp_meter_hidden) {
            system_show_cpu_temp_meter();
        }
        sys_cpu_temp_zero_start_time = 0; // Reset timer when value becomes non-zero
    }
    
    // ========================================================================
    // CPU LOAD METER LOGIC
    // ========================================================================
    
    if (cpu_load == 0) {
        // Load value is zero
        if (sys_last_cpu_load != 0) {
            // Just became zero, start the hiding timer
            sys_cpu_load_zero_start_time = current_time;
            Serial.println("CPU load became zero - starting timer");
        }
        // Check if it's been zero for more than the timeout period
        if (current_time - sys_cpu_load_zero_start_time >= METER_HIDE_TIMEOUT_MS) {
            system_hide_cpu_load_meter();
        }
    } else {
        // Load value is non-zero
        if (sys_cpu_load_meter_hidden) {
            system_show_cpu_load_meter();
        }
        sys_cpu_load_zero_start_time = 0; // Reset timer when value becomes non-zero
    }
    
    // Update last known values for next comparison
    sys_last_cpu_temp = cpu_temp;
    sys_last_cpu_load = cpu_load;
}

/**
 * @brief Check if meters should be hidden due to prolonged zero values
 * 
 * Performs periodic checks of the meter hiding conditions. This function is
 * called regularly from the main loop to ensure timely hiding of meters that
 * have been showing zero values for the configured timeout period.
 */
void system_check_meter_hiding_conditions() {
    unsigned long current_time = millis();
    
    // Check CPU temperature meter hiding condition
    if (sys_last_cpu_temp == 0 && sys_cpu_temp_zero_start_time > 0) {
        if (current_time - sys_cpu_temp_zero_start_time >= METER_HIDE_TIMEOUT_MS) {
            system_hide_cpu_temp_meter();
        }
    }
    
    // Check CPU load meter hiding condition
    if (sys_last_cpu_load == 0 && sys_cpu_load_zero_start_time > 0) {
        if (current_time - sys_cpu_load_zero_start_time >= METER_HIDE_TIMEOUT_MS) {
            system_hide_cpu_load_meter();
        }
    }
}

/**
 * @brief Hide CPU temperature meter
 * 
 * Coordinates the hiding of the CPU temperature meter by calling the appropriate
 * UI function and updating system state. Provides centralized control over
 * meter visibility with proper logging.
 */
void system_hide_cpu_temp_meter() {
    if (!sys_cpu_temp_meter_hidden) {
        ui_hide_cpu_temp_meter();  // Call UI function to hide the widget
        sys_cpu_temp_meter_hidden = true;
        Serial.println("CPU temperature meter hidden (zero for >1 minute)");
    }
}

/**
 * @brief Show CPU temperature meter
 * 
 * Coordinates the showing of the CPU temperature meter by calling the appropriate
 * UI function and updating system state. Provides centralized control over
 * meter visibility with proper logging.
 */
void system_show_cpu_temp_meter() {
    if (sys_cpu_temp_meter_hidden) {
        ui_show_cpu_temp_meter();  // Call UI function to show the widget
        sys_cpu_temp_meter_hidden = false;
        Serial.println("CPU temperature meter shown (non-zero data received)");
    }
}

/**
 * @brief Hide CPU load meter
 * 
 * Coordinates the hiding of the CPU load meter by calling the appropriate
 * UI function and updating system state. Provides centralized control over
 * meter visibility with proper logging.
 */
void system_hide_cpu_load_meter() {
    if (!sys_cpu_load_meter_hidden) {
        ui_hide_cpu_load_meter();  // Call UI function to hide the widget
        sys_cpu_load_meter_hidden = true;
        Serial.println("CPU load meter hidden (zero for >1 minute)");
    }
}

/**
 * @brief Show CPU load meter
 * 
 * Coordinates the showing of the CPU load meter by calling the appropriate
 * UI function and updating system state. Provides centralized control over
 * meter visibility with proper logging.
 */
void system_show_cpu_load_meter() {
    if (sys_cpu_load_meter_hidden) {
        ui_show_cpu_load_meter();  // Call UI function to show the widget
        sys_cpu_load_meter_hidden = false;
        Serial.println("CPU load meter shown (non-zero data received)");
    }
}

// ============================================================================
// DISPLAY POWER MANAGEMENT
// ============================================================================

/**
 * @brief Check for data timeout and blank display if needed
 * 
 * Monitors the time since last data reception and automatically blanks the
 * display after the configured timeout period. This conserves power and
 * provides visual feedback that the system has lost connection to the data source.
 */
void system_check_data_timeout() {
    unsigned long current_time = millis();
    
    // Only check timeout if we have received data before (avoid blanking during startup)
    if (sys_first_data_received && sys_last_data_received_time > 0) {
        unsigned long time_since_last_data = current_time - sys_last_data_received_time;
        
        // Check if no data has been received for more than the timeout period
        if (time_since_last_data >= DISPLAY_BLANK_TIMEOUT_MS) {
            system_blank_entire_display();
        }
    }
}

/**
 * @brief Blank entire display to save power
 * 
 * Coordinates complete display blanking by calling the appropriate UI function,
 * turning off the backlight, and updating system state. This provides a clear 
 * indication that the system has lost connection to the data source while 
 * conserving power.
 */
void system_blank_entire_display() {
    if (!sys_display_blanked) {
        ui_blank_entire_display();  // Call UI function to hide all elements
        backlight_off();             // Turn off backlight to save power
        sys_display_blanked = true;
        Serial.println("Display blanked - no data received for >1 minute");
    }
}

/**
 * @brief Restore display when new data arrives
 * 
 * Coordinates display restoration by calling the appropriate UI function,
 * turning on the backlight, and updating system state. Respects individual 
 * meter hiding states to ensure a consistent user experience.
 */
void system_show_entire_display() {
    if (sys_display_blanked) {
        ui_show_entire_display();  // Call UI function to show all elements
        backlight_on();             // Turn on backlight to restore visibility
        sys_display_blanked = false;
        Serial.println("Display restored - new data received");
    }
}

// ============================================================================
// SYSTEM MONITORING AND MAINTENANCE
// ============================================================================

/**
 * @brief Perform periodic system maintenance tasks
 * 
 * Handles all periodic system management tasks that need to be performed
 * regularly. This function should be called from the main loop to ensure
 * proper system operation.
 */
void system_periodic_update() {
    // Check if meters should be hidden due to prolonged zero values
    system_check_meter_hiding_conditions();
    
    // Check if display should be blanked due to data timeout
    system_check_data_timeout();
    
    // Additional periodic tasks can be added here as the system grows
}

/**
 * @brief Get system status information
 * 
 * Returns a formatted string containing current system state information
 * for debugging and monitoring purposes. Useful for troubleshooting and
 * system health monitoring.
 */
String system_get_status() {
    String status = "System Status:\n";
    status += "  First data received: " + String(sys_first_data_received ? "Yes" : "No") + "\n";
    status += "  Display blanked: " + String(sys_display_blanked ? "Yes" : "No") + "\n";
    status += "  CPU temp meter hidden: " + String(sys_cpu_temp_meter_hidden ? "Yes" : "No") + "\n";
    status += "  CPU load meter hidden: " + String(sys_cpu_load_meter_hidden ? "Yes" : "No") + "\n";
    status += "  Last CPU temp: " + String(sys_last_cpu_temp) + "\n";
    status += "  Last CPU load: " + String(sys_last_cpu_load) + "\n";
    
    if (sys_last_data_received_time > 0) {
        unsigned long time_since_data = millis() - sys_last_data_received_time;
        status += "  Time since last data: " + String(time_since_data) + "ms\n";
    } else {
        status += "  Time since last data: Never\n";
    }
    
    return status;
}
