#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*=========================================================
 * COLOR
 *=========================================================*/

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/*=========================================================
 * MEMORY
 *=========================================================*/

#define LV_USE_STDLIB_MALLOC LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB

#define LV_STDINT_INCLUDE   <stdint.h>
#define LV_STDDEF_INCLUDE   <stddef.h>
#define LV_STDBOOL_INCLUDE  <stdbool.h>
#define LV_INTTYPES_INCLUDE <inttypes.h>
#define LV_LIMITS_INCLUDE   <limits.h>
#define LV_STDARG_INCLUDE   <stdarg.h>

/*=========================================================
 * HAL
 *=========================================================*/

#define LV_DEF_REFR_PERIOD 16
#define LV_DPI_DEF 130

#define LV_TICK_CUSTOM 0

/*=========================================================
 * OS
 *=========================================================*/

#define LV_USE_OS LV_OS_NONE

/*=========================================================
 * DRAWING
 *=========================================================*/

#define LV_USE_DRAW_SW 1

#define LV_DRAW_SW_ASM LV_DRAW_SW_ASM_NONE

#define LV_USE_DRAW_VG_LITE 0
#define LV_USE_DRAW_OPENGLES 0

#define LV_DRAW_COMPLEX 1

/*=========================================================
 * LOG
 *=========================================================*/

#define LV_USE_LOG 0

/*=========================================================
 * ASSERT
 *=========================================================*/

#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0

/*=========================================================
 * FONTS
 *=========================================================*/

#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 0
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

#define LV_FONT_DEFAULT &lv_font_montserrat_14

#define LV_USE_FONT_SUBPX 0
#define LV_FONT_FMT_TXT_LARGE 0
#define LV_USE_FONT_COMPRESSED 0

/*=========================================================
 * TEXT
 *=========================================================*/

#define LV_TXT_ENC LV_TXT_ENC_UTF8

#define LV_TXT_BREAK_CHARS " ,.;:-_"

#define LV_TXT_LINE_BREAK_LONG_LEN 0

/*=========================================================
 * WIDGETS
 *=========================================================*/

#define LV_USE_ARC        0
#define LV_USE_BAR        0
#define LV_USE_BUTTON     1
#define LV_USE_BTN        1

#define LV_USE_CANVAS     0
#define LV_USE_CHECKBOX   0
#define LV_USE_DROPDOWN   0
#define LV_USE_IMAGE      1
#define LV_USE_IMG        1
#define LV_USE_LABEL      1
#define LV_USE_LINE       0
#define LV_USE_ROLLER     0
#define LV_USE_SLIDER     0
#define LV_USE_SWITCH     0
#define LV_USE_TEXTAREA   0
#define LV_USE_TABLE      0
#define LV_USE_CHART      0
#define LV_USE_CALENDAR   0
#define LV_USE_KEYBOARD   0
#define LV_USE_TABVIEW    0
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0
#define LV_USE_SPAN       0
#define LV_USE_SPINBOX    0
#define LV_USE_SPINNER    0
#define LV_USE_LIST       1
#define LV_USE_MENU       0
#define LV_USE_MSGBOX     0
#define LV_USE_COLORWHEEL 0

/*=========================================================
 * THEME
 *=========================================================*/

#define LV_USE_THEME_DEFAULT 0
#define LV_USE_THEME_BASIC 0
#define LV_USE_THEME_MONO 0

/*=========================================================
 * LAYOUT
 *=========================================================*/

#define LV_USE_FLEX 1
#define LV_USE_GRID 0

/*=========================================================
 * OTHER
 *=========================================================*/

#define LV_USE_SNAPSHOT 0
#define LV_USE_MONKEY 0
#define LV_USE_GRIDNAV 0
#define LV_USE_FRAGMENT 0
#define LV_USE_OBSERVER 0

/*=========================================================
 * DEVICES
 *=========================================================*/

#define LV_USE_SDL 0
#define LV_USE_LINUX_FBDEV 0

/*=========================================================
 * ALIGNMENT
 *=========================================================*/

#define LV_DRAW_BUF_ALIGN 4

#endif