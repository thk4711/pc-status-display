#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH           16
#define LV_COLOR_16_SWAP         0
#define LV_COLOR_SCREEN_TRANSP   0

/*=========================
   MEMORY SETTINGS
 *=========================*/
#define LV_MEM_CUSTOM            0
#define LV_MEM_SIZE              (48U * 1024U)
#define LV_MEM_BUF_MAX_NUM       16

/*====================
   HAL SETTINGS
 *====================*/
#define LV_DISP_DEF_REFR_PERIOD  30
#define LV_INDEV_DEF_READ_PERIOD 30
#define LV_TICK_CUSTOM           0
#define LV_DPI_DEF               130

/*=======================
 * FEATURE CONFIGURATION
 *=======================*/
#define LV_DRAW_COMPLEX          1
#define LV_SHADOW_CACHE_SIZE     0
#define LV_CIRCLE_CACHE_SIZE     4
#define LV_LAYER_SIMPLE_BUF_SIZE          (24 * 1024)
#define LV_LAYER_SIMPLE_FALLBACK_BUF_SIZE (3 * 1024)
#define LV_IMG_CACHE_DEF_SIZE    0
#define LV_GRADIENT_MAX_STOPS    2
#define LV_GRAD_CACHE_DEF_SIZE   0
#define LV_DITHER_GRADIENT       0
#define LV_DISP_ROT_MAX_BUF      (10 * 1024)

/*==================
 *   FONT USAGE
 *==================*/
#define LV_FONT_MONTSERRAT_8     0
#define LV_FONT_MONTSERRAT_10    0
#define LV_FONT_MONTSERRAT_12    0
#define LV_FONT_MONTSERRAT_14    1
#define LV_FONT_MONTSERRAT_16    1
#define LV_FONT_MONTSERRAT_18    1
#define LV_FONT_MONTSERRAT_20    1  /*<-- Enable this font*/
#define LV_FONT_MONTSERRAT_22    1
#define LV_FONT_MONTSERRAT_24    1
#define LV_FONT_MONTSERRAT_26    1
#define LV_FONT_MONTSERRAT_28    0
#define LV_FONT_MONTSERRAT_30    0
#define LV_FONT_MONTSERRAT_32    1
#define LV_FONT_MONTSERRAT_34    0
#define LV_FONT_MONTSERRAT_36    0
#define LV_FONT_MONTSERRAT_38    1
#define LV_FONT_MONTSERRAT_40    0
#define LV_FONT_MONTSERRAT_42    0
#define LV_FONT_MONTSERRAT_44    0
#define LV_FONT_MONTSERRAT_46    0
#define LV_FONT_MONTSERRAT_48    0
#define LV_FONT_DEFAULT          &lv_font_montserrat_14
#define LV_USE_FONT_PLACEHOLDER  1

/*=================
 *  TEXT SETTINGS
 *=================*/
#define LV_TXT_ENC               LV_TXT_ENC_UTF8
#define LV_TXT_BREAK_CHARS       " ,.;:-_"
#define LV_TXT_LINE_BREAK_LONG_LEN 0
#define LV_TXT_COLOR_CMD         "#"

/*==================
 *  WIDGET USAGE
 *==================*/
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  0
#define LV_USE_CANVAS     0
#define LV_USE_CHECKBOX   0
#define LV_USE_DROPDOWN   0
#define LV_USE_IMG        0
#define LV_USE_LABEL      1
#define LV_LABEL_TEXT_SELECTION 1
#define LV_LABEL_LONG_TXT_HINT  1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     1
#define LV_ROLLER_INF_PAGES 7
#define LV_USE_SLIDER     0
#define LV_USE_SWITCH     0
#define LV_USE_TEXTAREA   1
#define LV_TEXTAREA_DEF_PWD_SHOW_TIME 1500
#define LV_USE_TABLE      1

/*==================
 * EXTRA COMPONENTS
 *==================*/
#define LV_USE_ANIMIMG    0
#define LV_USE_CALENDAR   0
#define LV_CALENDAR_WEEK_STARTS_MONDAY 0
#define LV_USE_CALENDAR_HEADER_ARROW 0
#define LV_USE_CALENDAR_HEADER_DROPDOWN 0
#define LV_USE_CHART      1
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN     0
#define LV_USE_KEYBOARD   0
#define LV_USE_LED        0
#define LV_USE_LIST       0
#define LV_USE_MENU       0
#define LV_USE_METER      1
#define LV_USE_MSGBOX     0
#define LV_USE_SPAN       1
#define LV_SPAN_SNIPPET_STACK_SIZE 64
#define LV_USE_SPINBOX    1
#define LV_USE_SPINNER    1
#define LV_USE_TABVIEW    0
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0

/*-----------
 * Themes
 *----------*/
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0
#define LV_THEME_DEFAULT_GROW 1
#define LV_THEME_DEFAULT_TRANSITION_TIME 80
#define LV_USE_THEME_BASIC 1
#define LV_USE_THEME_MONO 1

/*-----------
 * Layouts
 *----------*/
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/*==================
* EXAMPLES
*==================*/
#define LV_BUILD_EXAMPLES 1

#endif /*LV_CONF_H*/
