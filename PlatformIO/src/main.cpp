/**
 * @file main.cpp
 * @brief Main application for ESP32-S3 system monitoring display
 * 
 * Real-time system monitoring display showing CPU temperature and load data
 * received via serial JSON messages. Features automatic UI management and
 * display power management.
 * 
 * @author ESP32-S3 Display Project
 * @date 2025
 */

#include <Arduino.h>
#include "display_driver.h"
#include "ui_components.h"
#include "system_manager.h"
#include <ArduinoJson.h>

// ============================================================================
// SYSTEM INITIALIZATION
// ============================================================================

/**
 * @brief System setup and initialization
 * 
 * Initializes serial communication, display hardware, UI components,
 * and system manager. Shows boot animation until first data received.
 */
void setup() {

  // Initialize serial communication for data reception
  Serial.begin(115200);
  
  // Wait for serial connection to be established (important for USB CDC)
  delay(1000);
  
  // Print startup information
  Serial.println("ESP32-S3 Display Project Starting...");
  Serial.println("Serial communication initialized at 115200 baud");
  
  // Initialize display hardware and LVGL graphics system
  Serial.println("Initializing display...");
  display_init();  // Set up SPI, GC9A01 panel, LVGL integration
  Serial.println("Display initialized successfully");
  
  // Initialize complete user interface system
  Serial.println("Initializing UI components...");
  ui_init();  // Create meters, labels, boot animation
  Serial.println("UI components initialized successfully");
  
  // Initialize system manager for system control logic
  Serial.println("Initializing system manager...");
  system_manager_init();  // Initialize system logic and state management
  Serial.println("System manager initialized successfully");
  
  // System ready - print usage information
  Serial.println("Setup complete - Ready to receive JSON data");
  Serial.println("Expected JSON format: {\"time\":\"HH:MM:SS\",\"cpu_load\":0-100,\"cpu_temp\":0-100}");
}

// ============================================================================
// MAIN APPLICATION LOOP
// ============================================================================

/**
 * @brief Main application loop - handles data processing and UI management
 * 
 * Processes incoming JSON data, updates UI elements, manages automatic
 * meter hiding/showing, and handles LVGL graphics processing.
 */
void loop() {
  // ========================================================================
  // SERIAL DATA PROCESSING
  // ========================================================================
  
  if (Serial.available()) {
    // Read complete JSON line from serial
    String json = Serial.readStringUntil('\n');
    
    // Parse JSON data with 256-byte buffer (sufficient for expected format)
    StaticJsonDocument<256> json_doc;
    DeserializationError error = deserializeJson(json_doc, json);
    
    if (!error) {
      // Extract data fields from JSON
      const char* timeStr = json_doc["time"];        // Time string "HH:MM:SS"
      int cpu_load = json_doc["cpu_load"];           // CPU load percentage 0-100
      int cpu_temperature = json_doc["cpu_temp"];    // CPU temperature 0-100°C
      
      // Process data through system manager (handles all system logic)
      system_process_data(cpu_temperature, cpu_load);
      
      // ====================================================================
      // UI UPDATES (only if display is active)
      // ====================================================================
      
      if (!sys_display_blanked) {
        // Update CPU temperature meter with smooth animation (only if visible)
        if (!sys_cpu_temp_meter_hidden) {
          update_meter_needle_animated(cpu_temp_meter, cpu_temperature, 600);
        }
        
        // Update CPU load meter with smooth animation (only if visible)
        if (!sys_cpu_load_meter_hidden) {
          update_meter_needle_animated(cpu_load_meter, cpu_load, 400);
        }
        
        // Always update time display when screen is active
        lv_label_set_text(time_label, timeStr);
      }
    }
    // Note: JSON parsing errors are silently ignored to maintain stability
  }

  // ========================================================================
  // SYSTEM MANAGEMENT
  // ========================================================================
  
  // Perform all periodic system management tasks
  system_periodic_update();

  // ========================================================================
  // GRAPHICS PROCESSING
  // ========================================================================
  
  // Process LVGL animations, timers, and screen updates
  lv_timer_handler();
  
  // Small delay for system stability and to prevent watchdog timeout
  delay(5);  // 5ms delay = ~200Hz loop frequency
}
