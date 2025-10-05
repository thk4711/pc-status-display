/**
 * @file ui_components.cpp
 * @brief Implementation of UI components for ESP32-S3 system monitoring display
 * 
 * This file implements the complete user interface system including analog meters,
 * boot animation, automatic hiding logic, and display power management. The UI
 * is designed for a 240x240 round LCD display and provides system monitoring
 * capabilities.
 * 
 * Key Implementation Features:
 * - Configurable analog meters with LVGL
 * - Windows-style boot animation with rotating arc
 * - Automatic meter hiding after 1 minute of zero values
 * - Display blanking after 1 minute of no data
 * - Efficient memory and processing management
 * - High-contrast color scheme and styling
 * 
 * @author ESP32-S3 Display Project
 * @date 2025
 */

#include "ui_components.h"
#include <Arduino.h>

// ============================================================================
// GLOBAL UI OBJECT HANDLES
// ============================================================================

lv_obj_t *cpu_temp_meter;              ///< CPU temperature analog meter widget
lv_obj_t *cpu_load_meter;              ///< CPU load analog meter widget
lv_obj_t *time_label;                  ///< Digital time display label
lv_obj_t *center_button;               ///< Central circular button (time background)

// ============================================================================
// BOOT ANIMATION SYSTEM VARIABLES
// ============================================================================

lv_obj_t *boot_animation_container = NULL; ///< Container for all boot animation elements
lv_obj_t *boot_spinner = NULL;             ///< Main rotating arc widget
lv_anim_t boot_anim;                       ///< Main rotation animation structure
// Note: first_data_received moved to system_manager.cpp for better architecture

// ============================================================================
// NEEDLE ANIMATION SYSTEM VARIABLES
// ============================================================================

lv_anim_t cpu_temp_needle_anim;            ///< Animation for CPU temperature needle
lv_anim_t cpu_load_needle_anim;            ///< Animation for CPU load needle

// Current needle position tracking for smooth animations
static int32_t cpu_temp_current_value = 30;  ///< Current CPU temperature needle position
static int32_t cpu_load_current_value = 30;  ///< Current CPU load needle position

// Note: System state variables moved to system_manager.cpp for better architecture

// ============================================================================
// INTERNAL VARIABLES
// ============================================================================

static int default_meter_value = 30;        ///< Default meter value for initialization

// ============================================================================
// CORE UI FUNCTIONS
// ============================================================================

/**
 * @brief Apply dark theme styling to the main screen
 * 
 * Sets the main screen background to pure black for maximum contrast and
 * clean appearance. This creates the foundation for the high-contrast
 * UI design with golden amber and red accents.
 */
void apply_dark_theme() {
  // Set pure black background for the main screen
  lv_obj_set_style_bg_color(lv_scr_act(), METER_BLACK, LV_PART_MAIN);
}

/**
 * @brief Apply visual styling to a meter widget based on configuration
 * 
 * Applies comprehensive styling to an LVGL meter widget using the provided
 * configuration structure. This includes background colors, transparency,
 * borders, and text colors for different meter parts.
 * 
 * @param meter Pointer to LVGL meter object to style
 * @param config Pointer to meter configuration containing style parameters
 */
void apply_meter_style(lv_obj_t *meter, const meter_config_t *config) {
    // Apply background styling
    lv_obj_set_style_bg_color(meter, config->colors.background, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(meter, LV_OPA_TRANSP, LV_PART_MAIN);  // Transparent background
    
    // Apply border styling
    lv_obj_set_style_border_width(meter, config->border_width, LV_PART_MAIN);
    
    // Apply text color for tick labels
    lv_obj_set_style_text_color(meter, config->colors.tick_labels, LV_PART_TICKS);
    
    // Optional: Add border color if border is enabled
    if (config->border_width > 0) {
        lv_obj_set_style_border_color(meter, config->colors.minor_ticks, LV_PART_MAIN);
    }
}

/**
 * @brief Create central button and time label widgets
 * 
 * Creates the central UI elements consisting of:
 * 1. A circular black button (130x130px) that serves as background
 * 2. A time label overlay with golden amber text
 * 
 * Button Styling:
 * - Size: 130x130 pixels (perfect circle)
 * - Color: Pure black to match theme
 * - Radius: 65px (half of size for perfect circle)
 * - No shadows, borders, or outlines for clean appearance
 * 
 * Time Label Styling:
 * - Font: Montserrat 32px (closest available to desired 20px)
 * - Color: Golden amber for visibility and theme consistency
 * - Background: Black with full opacity
 * - Alignment: Centered both horizontally and vertically
 * - Size: 100x60px with 5px padding
 */
void create_button_and_label() {
  // ========================================================================
  // CREATE CENTRAL CIRCULAR BUTTON
  // ========================================================================
  
  center_button = lv_btn_create(lv_scr_act());        // Create button on main screen
  lv_obj_set_size(center_button, 130, 130);          // Set to 130x130px square
  lv_obj_align(center_button, LV_ALIGN_CENTER, 0, 0); // Center on screen

  // Define custom style for perfectly circular black button
  static lv_style_t style_black_btn;
  lv_style_init(&style_black_btn);
  lv_style_set_bg_color(&style_black_btn, METER_BLACK);    // Pure black background
  lv_style_set_radius(&style_black_btn, 65);              // 65px radius = perfect circle
  lv_style_set_shadow_width(&style_black_btn, 0);         // No shadow for clean look
  lv_style_set_border_width(&style_black_btn, 0);         // No border
  lv_style_set_outline_width(&style_black_btn, 0);        // No outline

  // Apply the custom style to the button
  lv_obj_add_style(center_button, &style_black_btn, LV_PART_MAIN);

  // ========================================================================
  // CREATE TIME DISPLAY LABEL
  // ========================================================================
  
  time_label = lv_label_create(lv_scr_act());         // Create label on main screen
    
  // Set initial placeholder text
  lv_label_set_text(time_label, "16:24");
    
  // Configure label size and position
  lv_obj_set_size(time_label, 100, 60);               // 100x60px label area
  lv_obj_center(time_label);                          // Center on screen (over button)
    
  // Configure text alignment within label
  lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, 0);
    
  // Configure label background (black to match button)
  lv_obj_set_style_bg_color(time_label, METER_BLACK, 0);
  lv_obj_set_style_bg_opa(time_label, LV_OPA_COVER, 0);  // Full opacity background
    
  // Configure text appearance
  lv_obj_set_style_text_color(time_label, METER_GOLDEN_AMBER, 0);  // Golden amber text
  lv_obj_set_style_text_font(time_label, &lv_font_montserrat_32, 0); // 32px font (closest to desired 20px)
    
  // Add padding for better text positioning
  lv_obj_set_style_pad_all(time_label, 5, 0);
}


lv_obj_t* create_simple_meter_with_config(const meter_config_t *config) {
    // Create the meter gauge
    lv_obj_t *meter = lv_meter_create(lv_scr_act());
    lv_obj_center(meter);
    lv_obj_set_size(meter, config->width, config->height);

    // Apply styling
    apply_meter_style(meter, config);

    // Add a scale
    lv_meter_scale_t *scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_range(meter, scale, config->scale_min, config->scale_max, 
                            config->scale_angle, config->scale_rotation);
    
    // Set scale ticks
    lv_meter_set_scale_ticks(meter, scale, config->tick_count, config->tick_width, 
                            config->tick_length, config->colors.minor_ticks);
    lv_meter_set_scale_major_ticks(meter, scale, config->major_tick_every, 
                                  config->major_tick_width, config->major_tick_length, 
                                  config->colors.major_ticks, config->major_tick_label_gap);

    // Add green zone (cool zone)
    lv_meter_indicator_t *green_arc = lv_meter_add_arc(meter, scale, config->arc_width, 
                                                       config->colors.green_zone, 0);
    lv_meter_set_indicator_start_value(meter, green_arc, config->green_zone_start);
    lv_meter_set_indicator_end_value(meter, green_arc, config->green_zone_end);

    // Make the tick lines green at the start of the scale
    lv_meter_indicator_t *green_lines = lv_meter_add_scale_lines(meter, scale, 
                                                                config->colors.green_zone, 
                                                                config->colors.green_zone, 
                                                                false, 0);
    lv_meter_set_indicator_start_value(meter, green_lines, config->green_zone_start);
    lv_meter_set_indicator_end_value(meter, green_lines, config->green_zone_end);

    // Add red zone (hot zone)
    lv_meter_indicator_t *red_arc = lv_meter_add_arc(meter, scale, config->arc_width, 
                                                     config->colors.red_zone, 0);
    lv_meter_set_indicator_start_value(meter, red_arc, config->red_zone_start);
    lv_meter_set_indicator_end_value(meter, red_arc, config->red_zone_end);

    // Make the tick lines red at the end of the scale
    lv_meter_indicator_t *red_lines = lv_meter_add_scale_lines(meter, scale, 
                                                              config->colors.red_zone, 
                                                              config->colors.red_zone, 
                                                              false, 0);
    lv_meter_set_indicator_start_value(meter, red_lines, config->red_zone_start);
    lv_meter_set_indicator_end_value(meter, red_lines, config->red_zone_end);

    // Add needle line indicator
    lv_meter_indicator_t *needle_indicator = lv_meter_add_needle_line(meter, scale, 
                                                                     config->needle_width, 
                                                                     config->colors.needle, 
                                                                     config->needle_offset);
    lv_meter_set_indicator_value(meter, needle_indicator, default_meter_value);

    // Store the needle indicator in the meter's user data for later access
    lv_obj_set_user_data(meter, needle_indicator);

    return meter;
}

// Initialize all UI components
void ui_init() {
  apply_dark_theme();
  
  // Create main UI components
  cpu_temp_meter = create_simple_meter_with_config(&cpu_temp_meter_config);
  cpu_load_meter = create_simple_meter_with_config(&cpu_load_meter_config);
  create_button_and_label();
  
  // Initialize needle animation system
  init_needle_animations();
  
  // Initially hide main UI elements
  lv_obj_add_flag(cpu_temp_meter, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(cpu_load_meter, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(center_button, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(time_label, LV_OBJ_FLAG_HIDDEN);
  
  // Create and show boot animation
  create_boot_animation();
  show_boot_animation();
}


// ============================================================================
// NEEDLE ANIMATION SYSTEM
// ============================================================================

/**
 * @brief Animation callback for smooth needle movement
 * 
 * This callback function is called by LVGL during animation to update
 * the needle position with interpolated values between start and end.
 * 
 * @param meter_and_needle Pointer to structure containing meter and needle pointers
 * @param animated_value Current interpolated value during animation
 */
static void needle_animation_callback(void * meter_and_needle, int32_t animated_value) {
    // Cast the user data back to get meter and needle pointers
    lv_obj_t *meter = (lv_obj_t*)meter_and_needle;
    if (meter) {
        lv_meter_indicator_t *needle = (lv_meter_indicator_t*)lv_obj_get_user_data(meter);
        if (needle) {
            lv_meter_set_indicator_value(meter, needle, animated_value);
        }
    }
}

/**
 * @brief Initialize needle animation system
 * 
 * Sets up animation structures for both CPU temperature and load meters.
 * This prepares the animation system but doesn't start any animations.
 */
void init_needle_animations() {
    // Verify meters exist before setting up animations
    if (!cpu_temp_meter || !cpu_load_meter) {
        return;
    }
    
    // Initialize CPU temperature needle animation
    lv_anim_init(&cpu_temp_needle_anim);
    lv_anim_set_var(&cpu_temp_needle_anim, cpu_temp_meter);
    lv_anim_set_exec_cb(&cpu_temp_needle_anim, needle_animation_callback);
    lv_anim_set_path_cb(&cpu_temp_needle_anim, lv_anim_path_ease_out);  // Smooth easing
    
    // Initialize CPU load needle animation
    lv_anim_init(&cpu_load_needle_anim);
    lv_anim_set_var(&cpu_load_needle_anim, cpu_load_meter);
    lv_anim_set_exec_cb(&cpu_load_needle_anim, needle_animation_callback);
    lv_anim_set_path_cb(&cpu_load_needle_anim, lv_anim_path_ease_out);  // Smooth easing
}

/**
 * @brief Animation completion callback to update current value tracking
 * 
 * Called when needle animation completes to update the tracked current value.
 * This ensures the next animation starts from the correct position.
 * 
 * @param anim Pointer to the completed animation structure
 */
static void needle_animation_complete_callback(lv_anim_t * anim) {
    lv_obj_t *meter = (lv_obj_t*)anim->var;
    
    // The final value is already updated in our tracking variables
    // during the update_meter_needle_animated function, so no need
    // to update again here. This callback is just for cleanup if needed.
}

/**
 * @brief Update meter needle with smooth animation
 * 
 * Animates the needle from its current position to the new target value
 * over the specified duration. If an animation is already running, it
 * will be stopped and a new one started.
 * 
 * @param meter Pointer to meter widget
 * @param new_value Target value to animate to (0-100)
 * @param duration Animation duration in milliseconds
 */
void update_meter_needle_animated(lv_obj_t *meter, int32_t new_value, uint32_t duration) {
    if (!meter) return;
    
    lv_meter_indicator_t *needle = (lv_meter_indicator_t*)lv_obj_get_user_data(meter);
    if (!needle) return;
    
    // Get current needle value as starting point and determine animation structure
    int32_t current_value = 0;
    lv_anim_t *anim = NULL;
    
    if (meter == cpu_temp_meter) {
        anim = &cpu_temp_needle_anim;
        current_value = cpu_temp_current_value;
        // Update tracking for next animation
        cpu_temp_current_value = new_value;
    } else if (meter == cpu_load_meter) {
        anim = &cpu_load_needle_anim;
        current_value = cpu_load_current_value;
        // Update tracking for next animation
        cpu_load_current_value = new_value;
    }
    
    if (!anim) return;
    
    // Skip animation if the value hasn't changed
    if (current_value == new_value) return;
    
    // Stop any existing animation for this meter
    lv_anim_del(meter, needle_animation_callback);
    
    // Configure the animation
    lv_anim_set_var(anim, meter);
    lv_anim_set_values(anim, current_value, new_value);
    lv_anim_set_time(anim, duration);
    lv_anim_set_repeat_count(anim, 1);  // Run once
    lv_anim_set_ready_cb(anim, needle_animation_complete_callback);  // Update tracking when done
    
    // Start the animation
    lv_anim_start(anim);
}

// Boot animation callback function for rotation
static void boot_animation_callback(void * arc_widget, int32_t rotation_angle) {
    lv_obj_t * arc = (lv_obj_t *)arc_widget;
    // Rotate the 120-degree arc around the circle
    lv_arc_set_angles(arc, rotation_angle, rotation_angle + 120);
}

// Create the simple Windows-style boot animation
void create_boot_animation() {
    // Create container for boot animation
    boot_animation_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(boot_animation_container, LV_HOR_RES, LV_VER_RES);
    lv_obj_center(boot_animation_container);
    
    // Make container transparent with black background
    lv_obj_set_style_bg_color(boot_animation_container, METER_BLACK, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(boot_animation_container, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(boot_animation_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(boot_animation_container, 0, LV_PART_MAIN);
    
    // Create the spinning arc (similar to Windows boot animation)
    boot_spinner = lv_arc_create(boot_animation_container);
    lv_obj_set_size(boot_spinner, 130, 130);  // 130x130 size
    lv_obj_center(boot_spinner);
    
    // Configure arc appearance
    lv_arc_set_range(boot_spinner, 0, 360);
    lv_arc_set_value(boot_spinner, 0);
    lv_arc_set_bg_angles(boot_spinner, 0, 360);
    
    // Style the arc with thick lines
    lv_obj_set_style_arc_width(boot_spinner, 8, LV_PART_MAIN);  // 8px thick
    lv_obj_set_style_arc_color(boot_spinner, METER_BLACK, LV_PART_MAIN);
    
    lv_obj_set_style_arc_width(boot_spinner, 8, LV_PART_INDICATOR);  // 8px thick
    lv_obj_set_style_arc_color(boot_spinner, METER_GOLDEN_AMBER, LV_PART_INDICATOR);
    
    // Hide the knob (we don't want the draggable handle)
    lv_obj_set_style_bg_opa(boot_spinner, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_border_width(boot_spinner, 0, LV_PART_KNOB);
    lv_obj_set_style_pad_all(boot_spinner, 0, LV_PART_KNOB);
    
    // Remove background
    lv_obj_set_style_bg_opa(boot_spinner, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(boot_spinner, 0, LV_PART_MAIN);
    
    // Set initial arc to show 120-degree segment
    lv_arc_set_angles(boot_spinner, 0, 120);
    
    // Create the rotation animation for the 120-degree arc
    lv_anim_init(&boot_anim);
    lv_anim_set_var(&boot_anim, boot_spinner);
    lv_anim_set_exec_cb(&boot_anim, boot_animation_callback);
    lv_anim_set_time(&boot_anim, 2000); // 2 seconds per rotation
    lv_anim_set_repeat_count(&boot_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_values(&boot_anim, 0, 360);
    lv_anim_set_path_cb(&boot_anim, lv_anim_path_linear);
}

// Show boot animation
void show_boot_animation() {
    if (boot_animation_container) {
        lv_obj_clear_flag(boot_animation_container, LV_OBJ_FLAG_HIDDEN);
        lv_anim_start(&boot_anim);
    }
}

// Hide boot animation and show main UI
void hide_boot_animation() {
    if (boot_animation_container) {
        lv_anim_del(&boot_anim, NULL);
        lv_obj_add_flag(boot_animation_container, LV_OBJ_FLAG_HIDDEN);
        
        // Show main UI elements
        if (cpu_temp_meter) lv_obj_clear_flag(cpu_temp_meter, LV_OBJ_FLAG_HIDDEN);
        if (cpu_load_meter) lv_obj_clear_flag(cpu_load_meter, LV_OBJ_FLAG_HIDDEN);
        if (center_button) lv_obj_clear_flag(center_button, LV_OBJ_FLAG_HIDDEN);
        if (time_label) lv_obj_clear_flag(time_label, LV_OBJ_FLAG_HIDDEN);
    }
}

// ============================================================================
// UI-ONLY METER CONTROL FUNCTIONS
// ============================================================================

/**
 * @brief Hide CPU temperature meter widget (UI-only operation)
 * 
 * Pure UI function that only handles the LVGL widget visibility.
 * Called by system manager when system logic determines meter should be hidden.
 */
void ui_hide_cpu_temp_meter() {
    if (cpu_temp_meter) {
        lv_obj_add_flag(cpu_temp_meter, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Show CPU temperature meter widget (UI-only operation)
 * 
 * Pure UI function that only handles the LVGL widget visibility.
 * Called by system manager when system logic determines meter should be shown.
 */
void ui_show_cpu_temp_meter() {
    if (cpu_temp_meter) {
        lv_obj_clear_flag(cpu_temp_meter, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Hide CPU load meter widget (UI-only operation)
 * 
 * Pure UI function that only handles the LVGL widget visibility.
 * Called by system manager when system logic determines meter should be hidden.
 */
void ui_hide_cpu_load_meter() {
    if (cpu_load_meter) {
        lv_obj_add_flag(cpu_load_meter, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Show CPU load meter widget (UI-only operation)
 * 
 * Pure UI function that only handles the LVGL widget visibility.
 * Called by system manager when system logic determines meter should be shown.
 */
void ui_show_cpu_load_meter() {
    if (cpu_load_meter) {
        lv_obj_clear_flag(cpu_load_meter, LV_OBJ_FLAG_HIDDEN);
    }
}

// ============================================================================
// UI-ONLY DISPLAY CONTROL FUNCTIONS
// ============================================================================

/**
 * @brief Hide all UI elements (UI-only operation)
 * 
 * Pure UI function that only handles LVGL widget visibility.
 * Called by system manager when system logic determines display should be blanked.
 */
void ui_blank_entire_display() {
    // Hide all UI elements
    if (cpu_temp_meter) lv_obj_add_flag(cpu_temp_meter, LV_OBJ_FLAG_HIDDEN);
    if (cpu_load_meter) lv_obj_add_flag(cpu_load_meter, LV_OBJ_FLAG_HIDDEN);
    if (center_button) lv_obj_add_flag(center_button, LV_OBJ_FLAG_HIDDEN);
    if (time_label) lv_obj_add_flag(time_label, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief Show all UI elements (UI-only operation)
 * 
 * Pure UI function that only handles LVGL widget visibility.
 * Called by system manager when system logic determines display should be restored.
 * Note: System manager handles individual meter state logic separately.
 */
void ui_show_entire_display() {
    // Show all UI elements - system manager will handle individual meter states
    if (cpu_temp_meter) lv_obj_clear_flag(cpu_temp_meter, LV_OBJ_FLAG_HIDDEN);
    if (cpu_load_meter) lv_obj_clear_flag(cpu_load_meter, LV_OBJ_FLAG_HIDDEN);
    if (center_button) lv_obj_clear_flag(center_button, LV_OBJ_FLAG_HIDDEN);
    if (time_label) lv_obj_clear_flag(time_label, LV_OBJ_FLAG_HIDDEN);
}
