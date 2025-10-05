/**
 * @file display_driver.cpp
 * @brief Implementation of display driver for ESP32-S3 with GC9A01 round LCD
 * 
 * This file implements the DisplayDriver class and related functions for managing
 * the GC9A01 240x240 round LCD display connected to an ESP32-S3 microcontroller.
 * It provides hardware abstraction and LVGL integration for graphics rendering.
 * 
 * @author ESP32-S3 Display Project
 * @date 2025
 */

#include "display_driver.h"
#include <Arduino.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

hw_timer_t *lvgl_timer = NULL;  ///< Hardware timer for LVGL 1ms tick generation

// Display constants - GC9A01 is 240x240 round LCD
const uint16_t SCREEN_WIDTH = 240;   ///< Display width in pixels
const uint16_t SCREEN_HEIGHT = 240;  ///< Display height in pixels

DisplayDriver display;  ///< Global display driver instance

// ============================================================================
// DISPLAYDRIVER CLASS IMPLEMENTATION
// ============================================================================

/**
 * @brief DisplayDriver constructor - configures hardware for ESP32-S3 + GC9A01
 * 
 * Sets up complete hardware configuration for the GC9A01 round LCD display:
 * 
 * SPI Bus Configuration:
 * - Host: SPI2_HOST (ESP32-S3 compatible)
 * - Mode: 0 (CPOL=0, CPHA=0)
 * - Write Speed: 27MHz (safe maximum for stable operation)
 * - Read Speed: 16MHz (conservative for reliable data reading)
 * 
 * Pin Assignments (ESP32-S3 specific):
 * - SCLK (Clock): GPIO 1
 * - MOSI (Data): GPIO 2  
 * - DC (Data/Command): GPIO 4
 * - CS (Chip Select): GPIO 5
 * - RST (Reset): GPIO 3
 * - MISO: Not used (-1)
 * 
 * Panel Settings:
 * - RGB Order: false (BGR color order for GC9A01)
 * - Invert: true (required for proper color display)
 */
DisplayDriver::DisplayDriver(void) {
  { // Configure SPI bus parameters
    auto bus_config = bus.config();

    bus_config.spi_host   = SPI2_HOST;  // ESP32-S3 supports SPI2_HOST
    bus_config.spi_mode   = 0;          // SPI mode 0 (CPOL=0, CPHA=0)
    bus_config.freq_write = 27000000;   // 27MHz write speed (safe maximum)
    bus_config.freq_read  = 16000000;   // 16MHz read speed (conservative)
    bus_config.pin_sclk   = 1;          // Serial clock pin
    bus_config.pin_mosi   = 2;          // Master out, slave in (data)
    bus_config.pin_miso   = -1;         // Not used for display
    bus_config.pin_dc     = 4;          // Data/command selection pin

    bus.config(bus_config);
    panel.setBus(&bus);
  }

  { // Configure panel control pins and display settings
    auto panel_config = panel.config();

    panel_config.pin_cs   = 5;     // Chip select pin
    panel_config.pin_rst  = 3;     // Reset pin (hardware reset)
    panel_config.pin_busy = -1;    // Not used by GC9A01

    panel_config.rgb_order = false; // BGR color order (required for GC9A01)
    panel_config.invert = true;     // Invert colors (required for proper display)

    panel.config(panel_config);
  }

  setPanel(&panel);  // Register panel with LovyanGFX
}

// ============================================================================
// LVGL TIMER SYSTEM
// ============================================================================

/**
 * @brief Hardware timer interrupt handler for LVGL tick generation
 * 
 * This interrupt service routine (ISR) is called every 1ms by the ESP32-S3
 * hardware timer. It increments LVGL's internal tick counter, which is used
 * for animations, timeouts, and other time-based operations.
 * 
 * The IRAM_ATTR attribute ensures this function is stored in internal RAM
 * for fast execution during interrupts.
 */
void IRAM_ATTR lvgl_timer_interrupt_handler() {
  lv_tick_inc(1); // Increment LVGL tick by 1ms
}

/**
 * @brief Initialize hardware timer for LVGL tick generation
 * 
 * Sets up ESP32-S3 hardware timer to provide precise 1ms interrupts for LVGL.
 * This timing is critical for smooth animations and proper UI responsiveness.
 * 
 * Timer Configuration:
 * - Frequency: 1MHz (1 tick = 1 microsecond resolution)
 * - Alarm: 1000 microseconds (triggers every 1ms)
 * - Auto-reload: Enabled (continuous operation)
 * - Priority: 0 (default interrupt priority)
 */
void lvgl_timer_init() {
  // Create timer with 1MHz frequency (1 tick = 1 microsecond)
  lvgl_timer = timerBegin(1000000);
  
  // Attach interrupt service routine
  timerAttachInterrupt(lvgl_timer, &lvgl_timer_interrupt_handler);
  
  // Set alarm for 1000 microseconds (1ms), auto-reload enabled
  timerAlarm(lvgl_timer, 1000, true, 0);
  
  Serial.println("LVGL timer initialized (1ms tick)");
}

// ============================================================================
// LVGL DISPLAY INTEGRATION
// ============================================================================

/**
 * @brief LVGL display flush callback - transfers pixel data to physical display
 * 
 * This function is called by LVGL whenever a screen region needs to be updated.
 * It efficiently transfers pixel data from LVGL's internal buffer to the
 * physical display via SPI communication.
 * 
 * Process:
 * 1. Calculate update region dimensions
 * 2. Start SPI transaction for optimal performance
 * 3. Set display address window to target region
 * 4. Push pixel data in RGB565 format
 * 5. End SPI transaction
 * 6. Notify LVGL that flush is complete
 * 
 * @param disp Pointer to LVGL display driver structure
 * @param area Pointer to screen area that needs updating (x1,y1 to x2,y2)
 * @param color_p Pointer to pixel color data buffer in RGB565 format
 */
void display_flush_callback(lv_disp_drv_t *display_driver, const lv_area_t *update_area, lv_color_t *color_buffer) {
  // Calculate dimensions of update region
  int32_t w = update_area->x2 - update_area->x1 + 1;  // Width in pixels
  int32_t h = update_area->y2 - update_area->y1 + 1;  // Height in pixels

  // Perform efficient SPI transfer
  display.startWrite();                                           // Begin SPI transaction
  display.setAddrWindow(update_area->x1, update_area->y1, w, h); // Set target region
  display.pushPixels((lgfx::rgb565_t *)color_buffer, w * h);     // Transfer pixel data
  display.endWrite();                                            // End SPI transaction

  lv_disp_flush_ready(display_driver);  // Notify LVGL that flush is complete
}

// ============================================================================
// SYSTEM INITIALIZATION
// ============================================================================

/**
 * @brief Complete display and LVGL system initialization
 * 
 * Performs comprehensive initialization of the display system:
 * 
 * 1. Hardware Initialization:
 *    - Initialize DisplayDriver (SPI, pins, panel settings)
 *    - Set display rotation to 0° (portrait orientation)
 *    - Initialize backlight control (GPIO21)
 * 
 * 2. LVGL Library Setup:
 *    - Initialize LVGL core library
 *    - Set up hardware timer for 1ms ticks
 * 
 * 3. Memory Management:
 *    - Allocate DMA-capable buffer (10 lines = 2400 pixels)
 *    - Buffer size: 240px × 10 lines × 2 bytes = 4800 bytes
 *    - Uses DMA for optimal SPI transfer performance
 * 
 * 4. Display Driver Registration:
 *    - Configure LVGL display driver structure
 *    - Register flush callback for pixel data transfer
 *    - Set screen resolution (240×240)
 * 
 * This function must be called once during system startup before any
 * graphics operations can be performed.
 */
void display_init() {
  // Initialize the display hardware
  display.begin();         // Start DisplayDriver (SPI, panel initialization)
  display.setRotation(0);  // Set to portrait orientation (0°)

  // Initialize backlight control
  backlight_init();        // Configure GPIO21 and turn on backlight

  // Initialize LVGL graphics library
  lv_init();

  // Initialize hardware timer for LVGL (must be done early)
  lvgl_timer_init();

  // Allocate display buffer in DMA-capable memory for optimal performance
  // Buffer size: 240 pixels × 10 lines × 2 bytes/pixel = 4800 bytes
  static lv_disp_draw_buf_t draw_buf;
  static lv_color_t *buf = (lv_color_t *)heap_caps_malloc(
    SCREEN_WIDTH * 10 * sizeof(lv_color_t), 
    MALLOC_CAP_DMA  // DMA-capable memory for fast SPI transfers
  );
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, SCREEN_WIDTH * 10);

  // Register display driver with LVGL
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);              // Initialize with defaults
  disp_drv.flush_cb = display_flush_callback; // Set pixel transfer callback
  disp_drv.draw_buf = &draw_buf;             // Assign drawing buffer
  disp_drv.hor_res = SCREEN_WIDTH;           // Set horizontal resolution
  disp_drv.ver_res = SCREEN_HEIGHT;          // Set vertical resolution
  lv_disp_drv_register(&disp_drv);           // Register with LVGL
}

// ============================================================================
// BACKLIGHT CONTROL FUNCTIONS
// ============================================================================

/**
 * @brief Initialize backlight control GPIO
 * 
 * Configures GPIO21 as an output pin and turns on the backlight at startup.
 * This ensures the display is visible immediately after system initialization.
 */
void backlight_init() {
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH);  // Turn on backlight at startup
  Serial.println("Backlight initialized - GPIO21 configured and turned on");
}

/**
 * @brief Turn on display backlight
 * 
 * Sets GPIO21 HIGH to enable the display backlight. Used when restoring
 * the display after it has been blanked due to data timeout.
 */
void backlight_on() {
  digitalWrite(BACKLIGHT_PIN, HIGH);
  Serial.println("Backlight turned on");
}

/**
 * @brief Turn off display backlight
 * 
 * Sets GPIO21 LOW to disable the display backlight. Used when blanking
 * the display to save power when no data has been received for >1 minute.
 */
void backlight_off() {
  digitalWrite(BACKLIGHT_PIN, LOW);
  Serial.println("Backlight turned off");
}
