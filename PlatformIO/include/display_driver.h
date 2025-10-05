/**
 * @file display_driver.h
 * @brief Display driver for ESP32-S3 with GC9A01 round LCD display
 * 
 * Hardware abstraction for GC9A01 240x240 round LCD display connected
 * via SPI to ESP32-S3. Integrates with LVGL for graphics rendering.
 * 
 * @author ESP32-S3 Display Project
 * @date 2025
 */

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <LovyanGFX.hpp>
#ifndef LV_CONF_INCLUDE_SIMPLE
#define LV_CONF_INCLUDE_SIMPLE
#endif
#include "lv_conf.h"
#include <lvgl.h>

/**
 * @class DisplayDriver
 * @brief Hardware abstraction class for GC9A01 round LCD display
 * 
 * Extends LovyanGFX's LGFX_Device with GC9A01-specific configuration
 * including SPI setup, pin assignments, and panel settings.
 */
class DisplayDriver : public lgfx::LGFX_Device {
  lgfx::Panel_GC9A01 panel;  ///< GC9A01 panel configuration
  lgfx::Bus_SPI     bus;     ///< SPI bus configuration

public:
  /**
   * @brief Constructor - configures hardware parameters for ESP32-S3 + GC9A01
   */
  DisplayDriver(void);
};

// ============================================================================
// GLOBAL FUNCTIONS
// ============================================================================

/**
 * @brief Initialize the display hardware and LVGL graphics library
 */
void display_init();

/**
 * @brief Initialize backlight control GPIO
 */
void backlight_init();

/**
 * @brief Turn on display backlight
 */
void backlight_on();

/**
 * @brief Turn off display backlight
 */
void backlight_off();

/**
 * @brief Initialize hardware timer for LVGL tick generation
 */
void lvgl_timer_init();

/**
 * @brief LVGL display flush callback function
 * @param display_driver Pointer to LVGL display driver structure
 * @param update_area Pointer to screen area that needs updating
 * @param color_buffer Pointer to pixel color data buffer
 */
void display_flush_callback(lv_disp_drv_t *display_driver, const lv_area_t *update_area, lv_color_t *color_buffer);

// ============================================================================
// CONSTANTS
// ============================================================================

#define BACKLIGHT_PIN 6  ///< GPIO pin for display backlight control

extern const uint16_t SCREEN_WIDTH;   ///< Display width in pixels (240)
extern const uint16_t SCREEN_HEIGHT;  ///< Display height in pixels (240)

// ============================================================================
// GLOBAL INSTANCES
// ============================================================================

extern DisplayDriver display;  ///< Global display driver instance

#endif // DISPLAY_DRIVER_H
