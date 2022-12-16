/*
Process record user file (key pressed callback)

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

#include "process_record_user.h"
#include "enums.h"
#include QMK_KEYBOARD_H

uint8_t mods_state;


bool process_record_user(uint16_t keycode, keyrecord_t *record) {

  /* on key release */
  if (record->event.pressed == false) { 
    /* Luna pet */
    lunaIsJumping = false;
    lunaIsSneaking = false;
    lunaIsBarking = false;

    return true;            // Don't process keys further on key release (currently we don't need it)
  }  
  

  mods_state = get_mods();    // Store the current modifier state in the variable for later reference

  /* For OLED Key logger */
  // if (record->event.pressed) {
  //   // add_keylog(keycode);   
  // }

  
  switch (keycode) 
  {
    case KC_SPC:
      lunaIsJumping  = true;
      lunaShowedJump = false;
      break;

    case CTL_J:
    case CTL_F:  
      lunaIsSneaking = true;
      break;

    case ALT_K:
    case ALT_D:
      lunaIsBarking = true;
      break;

    case SHFT_AE:
      if (record->tap.count > 0) {    // The key is being tapped.
        tap_code16(ALGR(KC_Q));       // tapcode16() allows you to use modifiers!
        return false;
      }
      break;

    case CC_OE:
      tap_code16(ALGR(KC_P));
      break;

    case CC_UE:
      tap_code16(ALGR(KC_Y));   
      break;
        
    case CC_BckDel:
      if ( mods_state & MOD_MASK_SHIFT ) {  // Shift pressed
        del_mods(MOD_MASK_SHIFT);           // cancel the shifts so they are not applied to the keycodes
        tap_code(KC_DEL);                 
        set_mods(mods_state);
      }else {                               // No shift is pressed
        tap_code(KC_BSPC);
      }
      break;
    
    case CC_SLH:
      if ( mods_state & MOD_MASK_SHIFT ) {  
        del_mods(MOD_MASK_SHIFT);         
        tap_code(KC_BSLS);           // backslash   
        set_mods(mods_state);   
      }else {                               
        tap_code(KC_SLSH);          // /
      }
      break;

    case CC_APHO:
      if ( mods_state & MOD_MASK_SHIFT ) {  
        del_mods(MOD_MASK_SHIFT);         
        tap_code16(ALGR(KC_QUOT));     //  ´ aigu
        set_mods(mods_state);
           
      }else {                               
        tap_code(KC_GRV);              //  ` grave 
        tap_code(KC_SPC);              //  needed bc. of dead key
      }
      break;

    case SHFT_QT:
      if (record->tap.count > 0) {       // The key is being tapped.
        if ( mods_state & MOD_MASK_SHIFT ) {  
          tap_code(KC_QUOT);             // " 
          tap_code(KC_SPC);              //  needed bc. of dead key
        }else {                               
          tap_code(KC_QUOT);             // '  
          tap_code(KC_SPC);              //  needed bc. of dead key
        }
      }
      break;

    case CC_COLN:
      if ( mods_state & MOD_MASK_SHIFT ) {  
        //del_mods(MOD_MASK_SHIFT);         
        tap_code(KC_SCLN);             // :      
      }else {                               
        tap_code(KC_DOT);              // . (dot)
      }
      break;

    case CC_SCOLN:
      if ( mods_state & MOD_MASK_SHIFT ) {  
        del_mods(MOD_MASK_SHIFT);         
        tap_code(KC_SCLN);             // ; 
        set_mods(mods_state);    
      }else {                               
        tap_code(KC_COMM);             // , (comma)
      }
      break;

    case CC_EXLM:
      if ( mods_state & MOD_MASK_SHIFT ) {  
        del_mods(MOD_MASK_SHIFT);         
        tap_code16(KC_EXLM);             // !     
        set_mods(mods_state);
      }else {                               
        tap_code16(KC_QUES);             // ? 
      }
      break;
      
    #ifdef RGBLIGHT_ENABLE
    case CC_SAVRGB:   // Saves color config to eeprom. Used to save eeprom write cycles              
      rgblight_sethsv(rgblight_get_hue(), rgblight_get_sat(), rgblight_get_val()); 
      rgblight_set_speed( rgblight_get_speed() );
      break;

    case CC_ALRM:    // RGB alternate blinking
      rgblight_mode(RGBLIGHT_MODE_ALTERNATING);
      break;
    #endif
  }


  return true;
}