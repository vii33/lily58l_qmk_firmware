/*
SSD1306 OLED update loop, make sure to enable OLED_ENABLE=yes in rules.mk

Copyright 2012 Jun Wako <wakojun@gmail.com>
Copyright 2015 Jack Humbert
Copyright 2020 Ben Roesner (keycapsss.com)
Copyright 2022 Vii33 (https://github.com/vii33/mecha-keyboard-lily58l)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef OLED_ENABLE
#  include "oled.h"
#  include "enums.h"
#  include "process_record_user.h"

uint8_t current_wpm = 0;


oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    if (is_keyboard_master()) {
        return OLED_ROTATION_270;  
    } else {
        return OLED_ROTATION_0;
    }
}

/* OLED keylogging was disabled */
// #define  KEYLOG_LEN 6
// char     keylog_str[KEYLOG_LEN] = {};
// uint8_t  keylogs_str_idx        = 0;
// uint16_t log_timer              = 0;

// const char code_to_name[60] = {
//     ' ', ' ', ' ', ' ', 'a', 'b', 'c', 'd', 'e', 'f',
//     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
//     'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
//     '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
//     'R', 'E', 'B', 'T', '_', '-', '=', '[', ']', '\\',
//     '#', ';', '\'', '`', ',', '.', '/', ' ', ' ', ' '
// };

// void add_keylog(uint16_t keycode) {
//   if ((keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) || (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX)) {
//     keycode = keycode & 0xFF;
//   }

//   for (uint8_t i = KEYLOG_LEN - 1; i > 0; i--) {
//     keylog_str[i] = keylog_str[i - 1];
//   }

//   if (keycode < 60) {
//     keylog_str[0] = code_to_name[keycode];
//   }
//   keylog_str[KEYLOG_LEN - 1] = 0;

//   log_timer = timer_read();
// }

// void update_log(void) {
//   if (timer_elapsed(log_timer) > 750) {
//     add_keylog(0); // -> Gradually moves put in characters out of the image - needed for privacy reasons!
//   }
// }

// void render_keylogger_status(void) {
//     oled_write(keylog_str, false);
// }


void render_default_layer_state(void) {
  switch (get_highest_layer(layer_state)) {
    case _QWERTY:
        oled_write_P(PSTR("QWRTY"), false);
        break;
    case _LOWER:
        oled_write_P(PSTR("LOWER"), false);
        break;
    case _UPPER:
        oled_write_P(PSTR("UPPER"), false);
        break;
    case _MOD:
        oled_write_ln_P(PSTR("MOD"), true);
        break;
    case _GAME:
        oled_write_ln_P(PSTR("GAME"), true);
        break;
    default:
        oled_write_ln_P(PSTR("ndef"), true);
  }
}

void render_keylock_status(led_t led_state) {
  // oled_write_ln_P(PSTR("Lock"), false);
  oled_write_P(PSTR("  "), false);
  oled_write_P(PSTR("N"), led_state.num_lock);
  oled_write_P(PSTR("C"), led_state.caps_lock);
  oled_write_P(PSTR("S"), led_state.scroll_lock);
}

void render_mod_status(uint8_t modifiers) {
  // oled_write_ln_P(PSTR("Mods"), false);
  oled_write_P(PSTR(" "), false);
  oled_write_P(PSTR("S"), (modifiers & MOD_MASK_SHIFT));
  oled_write_P(PSTR("C"), (modifiers & MOD_MASK_CTRL));
  oled_write_P(PSTR("A"), (modifiers & MOD_MASK_ALT));
  oled_write_P(PSTR("G"), (modifiers & MOD_MASK_GUI));
}

void render_kb_status(void) {
  render_logo();
  oled_set_cursor(0, 3);
  oled_write_P(PSTR("-----"), false);   // Add a empty line
  
  render_default_layer_state();
  oled_write_P(PSTR("-----"), false);   // Add a empty line
  
  render_keylock_status(host_keyboard_led_state());
  oled_write_P(PSTR("-----"), false);
  
  render_mod_status(get_mods());
  oled_write_P(PSTR("-----"), false);

  // render_keylogger_status();

  /* Write words per minute counter */
  // oled_write_P(PSTR("WPM: "), false);
  // oled_write(get_u8_str(get_current_wpm(), '0'), false);
}



/* Callback for OLED update cycle */
/* ------------------------------ */
uint8_t current_frame = 0;
uint32_t anim_sleep_timer = 0;

bool oled_task_user(void) {
  current_wpm = get_current_wpm();   

  /* fixes screen on and off bug -> Turn on in process_record_user */
  if (current_wpm > 0) {
    oled_on();    
    anim_sleep_timer = timer_read32();
  } 
  else if ( timer_elapsed32(anim_sleep_timer) > OLED_TIMEOUT && is_oled_on() ) {
    oled_off();
    return false;
  }

  if (is_oled_on() == true) {     // Render enables screen, so only call it when screen is already on 
    // update_log();
    
    if (is_keyboard_master()) { 
      render_kb_status(); 
      render_luna(0, 13);
    } 
    else {
      render_right_oled();
    }
  }
  
  return false;
}
/* ------------------------------ */


/* ------ LUNA PET START ------ */

/* settings */
#    define MIN_WALK_SPEED      1
#    define MIN_RUN_SPEED       40
#    define ANIM_FRAME_DURATION 250  // how long each frame lasts in ms
#    define ANIM_SIZE           96   // number of bytes in array. If you change sprites, minimize for adequate firmware size. max is 1024

/* timers */
uint32_t anim_timer = 0;

bool lunaIsSneaking = false;
bool lunaIsJumping  = false;
bool lunaIsBarking  = false;
bool lunaShowedJump = true;

/* logic */
void render_luna(int LUNA_X, int LUNA_Y) {    // params: chunk position on LED font

    /* Sit */
    static const char PROGMEM sit[2][ANIM_SIZE] = {/* 'sit1', 32x22px */
                                                   {
                                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1c, 0x02, 0x05, 0x02, 0x24, 0x04, 0x04, 0x02, 0xa9, 0x1e, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x08, 0x68, 0x10, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x06, 0x82, 0x7c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0c, 0x10, 0x10, 0x20, 0x20, 0x20, 0x28, 0x3e, 0x1c, 0x20, 0x20, 0x3e, 0x0f, 0x11, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   },

                                                   /* 'sit2', 32x22px */
                                                   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1c, 0x02, 0x05, 0x02, 0x24, 0x04, 0x04, 0x02, 0xa9, 0x1e, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x90, 0x08, 0x18, 0x60, 0x10, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0x82, 0x7c, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0c, 0x10, 0x10, 0x20, 0x20, 0x20, 0x28, 0x3e, 0x1c, 0x20, 0x20, 0x3e, 0x0f, 0x11, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

    /* Walk */
    static const char PROGMEM walk[2][ANIM_SIZE] = {/* 'walk1', 32x22px */
                                                    {
                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x10, 0x90, 0x90, 0x90, 0xa0, 0xc0, 0x80, 0x80, 0x80, 0x70, 0x08, 0x14, 0x08, 0x90, 0x10, 0x10, 0x08, 0xa4, 0x78, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x08, 0xfc, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x18, 0xea, 0x10, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1c, 0x20, 0x20, 0x3c, 0x0f, 0x11, 0x1f, 0x03, 0x06, 0x18, 0x20, 0x20, 0x3c, 0x0c, 0x12, 0x1e, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                    },

                                                    /* 'walk2', 32x22px */
                                                    {
                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x20, 0x20, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x28, 0x10, 0x20, 0x20, 0x20, 0x10, 0x48, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x20, 0xf8, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x30, 0xd5, 0x20, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x20, 0x30, 0x0c, 0x02, 0x05, 0x09, 0x12, 0x1e, 0x02, 0x1c, 0x14, 0x08, 0x10, 0x20, 0x2c, 0x32, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                    }};

    /* Run */
    static const char PROGMEM run[2][ANIM_SIZE] = {/* 'run1', 32x22px */
                                                   {
                                                       0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x08, 0x08, 0xc8, 0xb0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40, 0x40, 0x3c, 0x14, 0x04, 0x08, 0x90, 0x18, 0x04, 0x08, 0xb0, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0xc4, 0xa4, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc8, 0x58, 0x28, 0x2a, 0x10, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x09, 0x04, 0x04, 0x04, 0x04, 0x02, 0x03, 0x02, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x10, 0x26, 0x2b, 0x32, 0x04, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   },

                                                   /* 'run2', 32x22px */
                                                   {
                                                       0x00, 0x00, 0x00, 0xe0, 0x10, 0x10, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x78, 0x28, 0x08, 0x10, 0x20, 0x30, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x08, 0x10, 0x11, 0xf9, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0xb0, 0x50, 0x55, 0x20, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x0c, 0x10, 0x20, 0x28, 0x37, 0x02, 0x1e, 0x20, 0x20, 0x18, 0x0c, 0x14, 0x1e, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                   }};

    /* Bark */
    static const char PROGMEM bark[2][ANIM_SIZE] = {/* 'bark1', 32x22px */
                                                    {
                                                        0x00, 0xc0, 0x20, 0x10, 0xd0, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40, 0x3c, 0x14, 0x04, 0x08, 0x90, 0x18, 0x04, 0x08, 0xb0, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x08, 0x10, 0x11, 0xf9, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc8, 0x48, 0x28, 0x2a, 0x10, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x0c, 0x10, 0x20, 0x28, 0x37, 0x02, 0x02, 0x04, 0x08, 0x10, 0x26, 0x2b, 0x32, 0x04, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                    },

                                                    /* 'bark2', 32x22px */
                                                    {
                                                        0x00, 0xe0, 0x10, 0x10, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x40, 0x40, 0x2c, 0x14, 0x04, 0x08, 0x90, 0x18, 0x04, 0x08, 0xb0, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x08, 0x10, 0x11, 0xf9, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x48, 0x28, 0x2a, 0x10, 0x0f, 0x20, 0x4a, 0x09, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x0c, 0x10, 0x20, 0x28, 0x37, 0x02, 0x02, 0x04, 0x08, 0x10, 0x26, 0x2b, 0x32, 0x04, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                    }};

    /* Sneak */
    static const char PROGMEM sneak[2][ANIM_SIZE] = {/* 'sneak1', 32x22px */
                                                     {
                                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x40, 0x40, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x40, 0x40, 0x80, 0x00, 0x80, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x21, 0xf0, 0x04, 0x02, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x04, 0x04, 0x04, 0x03, 0x01, 0x00, 0x00, 0x09, 0x01, 0x80, 0x80, 0xab, 0x04, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1c, 0x20, 0x20, 0x3c, 0x0f, 0x11, 0x1f, 0x02, 0x06, 0x18, 0x20, 0x20, 0x38, 0x08, 0x10, 0x18, 0x04, 0x04, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00,
                                                     },

                                                     /* 'sneak2', 32x22px */
                                                     {
                                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x40, 0x40, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xa0, 0x20, 0x40, 0x80, 0xc0, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x41, 0xf0, 0x04, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x04, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x40, 0x40, 0x55, 0x82, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x20, 0x30, 0x0c, 0x02, 0x05, 0x09, 0x12, 0x1e, 0x04, 0x18, 0x10, 0x08, 0x10, 0x20, 0x28, 0x34, 0x06, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                     }};

    // current_wpm   = get_current_wpm(); 
    
    /* animation timer */
    if (timer_elapsed32(anim_timer) > ANIM_FRAME_DURATION) {
      anim_timer = timer_read32();
              
      /* jump */
      if (lunaIsJumping || !lunaShowedJump) {
          /* clear */
          oled_set_cursor(LUNA_X, LUNA_Y + 2);
          oled_write("     ", false);

          oled_set_cursor(LUNA_X, LUNA_Y - 1);

          lunaShowedJump = true;

      } else {
          /* clear */
          oled_set_cursor(LUNA_X, LUNA_Y - 1);
          oled_write("     ", false);

          oled_set_cursor(LUNA_X, LUNA_Y);
      }

      /* switch frame */
      current_frame = (current_frame + 1) % 2;

      /* current status */
      if (lunaIsBarking) {
          oled_write_raw_P(bark[abs(1 - current_frame)], ANIM_SIZE);

      } else if (lunaIsSneaking) {
          oled_write_raw_P(sneak[abs(1 - current_frame)], ANIM_SIZE);

      } else if (current_wpm <= MIN_WALK_SPEED) {
          oled_write_raw_P(sit[abs(1 - current_frame)], ANIM_SIZE);

      } else if (current_wpm <= MIN_RUN_SPEED) {
          oled_write_raw_P(walk[abs(1 - current_frame)], ANIM_SIZE);

      } else {
          oled_write_raw_P(run[abs(1 - current_frame)], ANIM_SIZE);
      }
    }
}
/* ------ LUNA PET END ------ */


/* Remders Lily58 logo. For different animations see https://github.com/vii33/mecha-keyboard-lily58l */
void render_right_oled(void) {
  if (timer_elapsed32(anim_timer) > ANIM_FRAME_DURATION) {
    anim_timer = timer_read32();

    /* switch frame */
    current_frame = (current_frame + 1) % 2;   

    static const char PROGMEM lily58_logo[2][512] = {
        /* frame 1, 32x128px */
        {
            0,0,0,0,0,0,64,192,64,64,192,128,128,128,0,0,128,224,112,60,14,6,14,60,112,224,128,0,0,192,192,0,192,192,0,192,192,0,0,0,0,0,0,0,192,192,0,0,0,0,0,0,0,0,0,0,0,128,192,128,0,0,0,128,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,192,192,192,192,192,192,192,192,192,128,0,0,0,0,0,0,128,128,192,192,192,192,192,128,128,0,0,0,0,0,0,0,192,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,252,192,128,128,128,129,131,131,7,7,12,24,112,224,128,0,0,1,255,252,128,182,182,128,176,176,0,54,54,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,240,241,0,0,0,0,255,255,0,0,0,48,240,240,128,0,0,0,0,0,128,240,240,48,0,0,127,255,255,225,113,113,241,241,225,193,129,0,0,0,0,12,63,255,243,225,193,193,129,129,195,255,127,28,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,32,112,120,220,204,134,6,3,3,1,1,1,1,1,1,1,1,3,2,6,132,225,251,56,28,12,2,1,1,1,1,1,1,1,3,3,6,134,204,220,120,112,32,0,255,255,128,128,128,128,128,128,128,128,128,0,0,255,255,0,0,0,0,255,255,128,0,0,0,0,3,31,126,248,224,240,126,31,3,0,0,0,0,224,224,192,192,128,128,128,192,225,255,127,63,0,0,0,62,255,255,193,192,128,129,129,195,195,255,254,60,0,0,0,0,0,255,255,128,128,128,128,128,128,128,128,128,0,0,0,0,0,1,1,3,3,6,6,6,4,4,4,4,6,6,2,3,1,1,0,1,1,3,2,6,6,4,4,4,4,6,6,6,3,3,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,1,1,1,0,0,96,96,112,56,31,15,3,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,
        },
        /* frame 2, 32x128px */
        {  
            0,0,0,0,0,0,64,192,64,64,192,128,128,128,0,0,128,224,112,60,14,6,14,60,112,224,128,0,0,0,0,0,192,192,0,0,0,0,0,0,0,0,0,0,192,192,0,0,0,0,0,0,0,0,0,0,0,128,192,128,0,0,0,128,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,192,192,192,192,192,192,192,192,192,128,0,0,0,0,0,0,128,128,192,192,192,192,192,128,128,0,0,0,0,0,0,0,192,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63,252,192,128,128,128,129,131,131,7,7,12,24,112,224,128,0,0,1,255,252,128,134,134,128,182,182,0,6,6,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,240,241,0,0,0,0,255,255,0,0,0,48,240,240,128,0,0,0,0,0,128,240,240,48,0,0,127,255,255,225,113,113,241,241,225,193,129,0,0,0,0,12,63,255,243,225,193,193,129,129,195,255,127,28,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,32,112,120,220,204,134,6,3,3,1,1,1,1,1,1,1,1,3,2,6,132,225,251,56,28,12,2,1,1,1,1,1,1,1,3,3,6,134,204,220,120,112,32,0,255,255,128,128,128,128,128,128,128,128,128,0,0,255,255,0,0,0,0,255,255,128,0,0,0,0,3,31,126,248,224,240,126,31,3,0,0,0,0,224,224,192,192,128,128,128,192,225,255,127,63,0,0,0,62,255,255,193,192,128,129,129,195,195,255,254,60,0,0,0,0,0,255,255,128,128,128,128,128,128,128,128,128,0,0,0,0,0,1,1,3,3,6,6,6,4,4,4,4,6,6,2,3,1,1,0,1,1,3,2,6,6,4,4,4,4,6,6,6,3,3,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,1,1,1,0,0,96,96,112,56,31,15,3,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,
        }
    };

    oled_write_raw_P(lily58_logo[abs(1 - current_frame)], 512);
  }
}


/* vii33 logo.  32x8px */
void render_logo(void) {
    static const char PROGMEM vii33_logo[] = {
        0, 15, 48, 64, 48, 15,  0, 65,127, 65,  0, 65,127, 65,  0, 34, 65, 73, 73, 54,  0, 34, 65, 73, 73, 54,  0, 96, 24,  6,  1,  0,
    };
    oled_write_raw_P(vii33_logo, sizeof(vii33_logo));
}


#endif // OLED_ENABLE


