/**
 * @file ui_components.h
 * @brief User Interface components for ESP32-S3 system monitoring display
 * 
 * Complete UI system for displaying real-time system metrics on round LCD.
 * Features dual analog meters with automatic hiding/showing, boot animation,
 * and configurable themes.
 * 
 * @author ESP32-S3 Display Project
 * @date 2025
 */

#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H

#include <lvgl.h>

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/**
 * @struct meter_colors_t
 * @brief Color scheme definition for analog meters
 */
typedef struct {
    lv_color_t background;   ///< Meter background color
    lv_color_t tick_labels;  ///< Color for numeric labels on scale
    lv_color_t minor_ticks;  ///< Color for minor tick marks
    lv_color_t major_ticks;  ///< Color for major tick marks
    lv_color_t green_zone;   ///< Color for safe/normal operating zone
    lv_color_t red_zone;     ///< Color for warning/danger zone
    lv_color_t needle;       ///< Color for meter needle/pointer
} meter_colors_t;

/**
 * @struct meter_config_t
 * @brief Complete configuration structure for analog meters
 */
typedef struct {
    // Size and position
    lv_coord_t width;         ///< Meter widget width in pixels
    lv_coord_t height;        ///< Meter widget height in pixels
    
    // Scale configuration
    int32_t scale_min;        ///< Minimum scale value (typically 0)
    int32_t scale_max;        ///< Maximum scale value (typically 100)
    uint16_t scale_angle;     ///< Arc span in degrees (160° for partial circle)
    uint16_t scale_rotation;  ///< Starting angle offset (190° for top meter, 10° for bottom)
    
    // Tick configuration
    uint16_t tick_count;      ///< Total number of tick marks (21 for 0-100 in steps of 5)
    uint16_t tick_width;      ///< Width of minor tick marks in pixels
    uint16_t tick_length;     ///< Length of minor tick marks in pixels
    uint16_t major_tick_every;///< Every Nth tick becomes major (4 = every 20 units)
    uint16_t major_tick_width;///< Width of major tick marks in pixels
    uint16_t major_tick_length;///< Length of major tick marks in pixels
    int16_t major_tick_label_gap;///< Gap between major ticks and labels
    
    // Zone configuration
    int32_t green_zone_start; ///< Start value of safe zone (0 for disabled)
    int32_t green_zone_end;   ///< End value of safe zone (0 for disabled)
    int32_t red_zone_start;   ///< Start value of warning zone (80 for temp)
    int32_t red_zone_end;     ///< End value of warning zone (100 for temp)
    
    // Arc and needle configuration
    lv_coord_t arc_width;     ///< Width of colored zone arcs in pixels
    lv_coord_t needle_width;  ///< Width of meter needle in pixels
    int16_t needle_offset;    ///< Needle length offset from center (-10 for shorter)
    
    // Border configuration
    lv_coord_t border_width;  ///< Meter border width (0 for no border)
    
    // Colors
    meter_colors_t colors;    ///< Complete color scheme for this meter
} meter_config_t;

// ============================================================================
// HELPER MACROS
// ============================================================================

/**
 * @brief Helper macro to create LVGL color from RGB values
 */
#define METER_COLOR(r, g, b) lv_color_make(r, g, b)

#define METER_BLACK         METER_COLOR(0, 0, 0)        ///< Pure black background
#define METER_WHITE         METER_COLOR(255, 255, 255)  ///< Pure white for needles
//#define METER_GOLDEN_AMBER  METER_COLOR(181, 166, 66)   ///< Warm amber for UI elements
#define METER_GOLDEN_AMBER  METER_COLOR(255, 130, 3)   ///< Warm amber for UI elements
#define METER_BRIGHT_RED    METER_COLOR(255, 50, 50)    ///< Alert red for warnings

// ============================================================================
// METER CONFIGURATIONS
// ============================================================================

/**
 * @brief CPU Temperature Meter Configuration
 */
static const meter_config_t cpu_temp_meter_config = {
    .width = 235,
    .height = 235,
    
    .scale_min = 0,
    .scale_max = 100,
    .scale_angle = 160,
    .scale_rotation = 190,
    
    .tick_count = 21,
    .tick_width = 2,
    .tick_length = 10,
    .major_tick_every = 4,
    .major_tick_width = 4,
    .major_tick_length = 15,
    .major_tick_label_gap = 10,
    
    .green_zone_start = 0,
    .green_zone_end = 0,
    .red_zone_start = 80,
    .red_zone_end = 100,
    
    .arc_width = 3,
    .needle_width = 4,
    .needle_offset = -10,
    
    .border_width = 0,
    
    .colors = {
        .background = METER_BLACK,
        .tick_labels = METER_GOLDEN_AMBER,
        .minor_ticks = METER_GOLDEN_AMBER,
        .major_ticks = METER_GOLDEN_AMBER,
        .green_zone = METER_GOLDEN_AMBER,
        .red_zone = METER_BRIGHT_RED,
        .needle = METER_WHITE
    }
};

/**
 * @brief CPU Load Meter Configuration
 */
static const meter_config_t cpu_load_meter_config = {
        .width = 235,
        .height = 235,
        
        .scale_min = 0,
        .scale_max = 100,
        .scale_angle = 160,
        .scale_rotation = 10,
        
        .tick_count = 21,
        .tick_width = 2,
        .tick_length = 10,
        .major_tick_every = 4,
        .major_tick_width = 4,
        .major_tick_length = 15,
        .major_tick_label_gap = 10,
        
        .green_zone_start = 0,
        .green_zone_end = 00,
        .red_zone_start = 100,
        .red_zone_end = 100,
    
    .arc_width = 3,
    .needle_width = 4,
    .needle_offset = -10,
    
    .border_width = 0,
    
    .colors = {
        .background = METER_BLACK,
        .tick_labels = METER_GOLDEN_AMBER,
        .minor_ticks = METER_GOLDEN_AMBER,
        .major_ticks = METER_GOLDEN_AMBER,
        .green_zone = METER_GOLDEN_AMBER,
        .red_zone = METER_GOLDEN_AMBER,
        .needle = METER_WHITE
    }
};


// ============================================================================
// GLOBAL UI OBJECTS
// ============================================================================

extern lv_obj_t *cpu_temp_meter;  ///< CPU temperature analog meter widget
extern lv_obj_t *cpu_load_meter;  ///< CPU load analog meter widget  
extern lv_obj_t *center_button;   ///< Central circular button (time display background)
extern lv_obj_t *time_label;      ///< Digital time display label

// ============================================================================
// NEEDLE ANIMATION SYSTEM
// ============================================================================

extern lv_anim_t cpu_temp_needle_anim;  ///< Animation for CPU temperature needle
extern lv_anim_t cpu_load_needle_anim;  ///< Animation for CPU load needle

// ============================================================================
// BOOT ANIMATION SYSTEM
// ============================================================================

extern lv_obj_t *boot_animation_container;  ///< Container for boot animation elements
extern lv_obj_t *boot_spinner;             ///< Main spinning arc widget
extern lv_anim_t boot_anim;                 ///< Main rotation animation

// Note: System state variables moved to system_manager.h for better architecture

// ============================================================================
// CORE UI FUNCTIONS
// ============================================================================

/**
 * @brief Apply visual styling to a meter widget based on configuration
 * @param meter Pointer to LVGL meter object
 * @param config Pointer to meter configuration structure
 */
void apply_meter_style(lv_obj_t *meter, const meter_config_t *config);

/**
 * @brief Create a complete analog meter widget with specified configuration
 * @param config Pointer to meter configuration structure
 * @return lv_obj_t* Pointer to created meter widget
 */
lv_obj_t* create_simple_meter_with_config(const meter_config_t *config);

/**
 * @brief Create central button and time label widgets
 */
void create_button_and_label();

/**
 * @brief Initialize complete UI system
 */
void ui_init();

/**
 * @brief Apply dark theme styling to the main screen
 */
void apply_dark_theme();

/**
 * @brief Update meter needle position to display new value
 * @param meter Pointer to meter widget
 * @param value New value to display (0-100)
 */
void update_simple_meter_needle(lv_obj_t *meter, int32_t value);

// ============================================================================
// NEEDLE ANIMATION FUNCTIONS
// ============================================================================

/**
 * @brief Update meter needle with smooth animation
 * @param meter Pointer to meter widget
 * @param new_value Target value to animate to (0-100)
 * @param duration Animation duration in milliseconds (default: 500ms)
 */
void update_meter_needle_animated(lv_obj_t *meter, int32_t new_value, uint32_t duration);

/**
 * @brief Initialize needle animation system
 */
void init_needle_animations();

// ============================================================================
// BOOT ANIMATION SYSTEM
// ============================================================================

/**
 * @brief Create Windows-style boot animation
 */
void create_boot_animation();

/**
 * @brief Show boot animation and start rotation
 */
void show_boot_animation();

/**
 * @brief Hide boot animation and show main UI
 */
void hide_boot_animation();

// ============================================================================
// UI-ONLY METER CONTROL FUNCTIONS
// ============================================================================

/**
 * @brief Hide CPU temperature meter widget (UI-only operation)
 */
void ui_hide_cpu_temp_meter();

/**
 * @brief Show CPU temperature meter widget (UI-only operation)
 */
void ui_show_cpu_temp_meter();

/**
 * @brief Hide CPU load meter widget (UI-only operation)
 */
void ui_hide_cpu_load_meter();

/**
 * @brief Show CPU load meter widget (UI-only operation)
 */
void ui_show_cpu_load_meter();

// ============================================================================
// UI-ONLY DISPLAY CONTROL FUNCTIONS
// ============================================================================

/**
 * @brief Hide all UI elements (UI-only operation)
 */
void ui_blank_entire_display();

/**
 * @brief Show all UI elements (UI-only operation)
 */
void ui_show_entire_display();

#endif // UI_COMPONENTS_H
