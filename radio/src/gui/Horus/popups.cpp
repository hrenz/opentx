/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "../../opentx.h"

const char *s_warning = NULL;
const char *s_warning_info;
uint8_t     s_warning_info_len;
uint8_t     s_warning_type;
uint8_t     s_warning_result = 0;
uint8_t     s_warning_info_flags = ZCHAR;
int16_t     s_warning_input_value;
int16_t     s_warning_input_min;
int16_t     s_warning_input_max;

void displayAlertBox()
{
  lcdClear();
  lcdDrawSolidFilledRect(POPUP_X, POPUP_Y, POPUP_W, POPUP_H, TEXT_BGCOLOR);
  lcdDrawSolidRect(POPUP_X, POPUP_Y, POPUP_W, POPUP_H, ALARM_COLOR);
  lcdDrawSolidRect(POPUP_X+1, POPUP_Y+1, POPUP_W-2, POPUP_H-2, ALARM_COLOR);
  lcdDrawBitmap(POPUP_X-80, POPUP_Y-30, LBM_ASTERISK);
}

void displayWarningBox()
{
  lcdDrawSolidFilledRect(POPUP_X, POPUP_Y, POPUP_W, POPUP_H, TEXT_BGCOLOR);
  lcdDrawSolidRect(POPUP_X, POPUP_Y, POPUP_W, POPUP_H, ALARM_COLOR);
  lcdDrawSolidRect(POPUP_X+1, POPUP_Y+1, POPUP_W-2, POPUP_H-2, ALARM_COLOR);
  // lcdDrawBitmap(POPUP_X+15, POPUP_Y+20, LBM_WARNING);
}

void displayMessageBox()
{
  lcdDrawSolidFilledRect(POPUP_X, POPUP_Y, POPUP_W, POPUP_H, TEXT_BGCOLOR);
  lcdDrawSolidRect(POPUP_X, POPUP_Y, POPUP_W, POPUP_H, WARNING_COLOR);
  lcdDrawSolidRect(POPUP_X+1, POPUP_Y+1, POPUP_W-2, POPUP_H-2, WARNING_COLOR);
  // lcdDrawBitmap(POPUP_X+15, POPUP_Y+20, LBM_MESSAGE);
}

void message(const pm_char *title, const pm_char *t, const char *last MESSAGE_SOUND_ARG)
{
  displayAlertBox();

#if defined(TRANSLATIONS_FR) || defined(TRANSLATIONS_IT) || defined(TRANSLATIONS_CZ)
  lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y, STR_WARNING, ALARM_COLOR|DBLSIZE);
  lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y+25, title, ALARM_COLOR|DBLSIZE);
#else
  lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y, title, ALARM_COLOR|DBLSIZE);
  lcdDrawText(WARNING_LINE_X, WARNING_LINE_Y+25, STR_WARNING, ALARM_COLOR|DBLSIZE);
#endif

  if (t) lcdDrawText(WARNING_LINE_X, WARNING_INFOLINE_Y, t);
  if (last) {
    lcdDrawText(WARNING_LINE_X, WARNING_INFOLINE_Y+16, last);
    AUDIO_ERROR_MESSAGE(sound);
  }

  lcdRefresh();
  lcdSetContrast();
  clearKeyEvents();
}

void displayPopup(const char *title)
{
  // displayMessageBox();
  lcdDrawTextWithLen(WARNING_LINE_X, WARNING_LINE_Y, title, WARNING_LINE_LEN, DBLSIZE|WARNING_COLOR);
  lcdRefresh();
}

void displayWarning(evt_t event)
{
  s_warning_result = false;
  if (s_warning_type == WARNING_TYPE_INPUT)
    displayMessageBox();
  else
    displayWarningBox();
  lcdDrawTextWithLen(WARNING_LINE_X, WARNING_LINE_Y, s_warning, WARNING_LINE_LEN, DBLSIZE | (s_warning_type == WARNING_TYPE_INPUT ? WARNING_COLOR : ALARM_COLOR));
  if (s_warning_info) {
    lcdDrawTextWithLen(WARNING_LINE_X, WARNING_INFOLINE_Y, s_warning_info, s_warning_info_len, WARNING_INFO_FLAGS);
  }
  lcdDrawText(WARNING_LINE_X, WARNING_INFOLINE_Y+20, s_warning_type == WARNING_TYPE_ASTERISK ? STR_EXIT : STR_POPUPS);
  switch (event) {
    case EVT_KEY_BREAK(KEY_ENTER):
      if (s_warning_type == WARNING_TYPE_ASTERISK)
        break;
      s_warning_result = true;
      // no break
    case EVT_KEY_BREAK(KEY_EXIT):
      s_warning = NULL;
      s_warning_type = WARNING_TYPE_ASTERISK;
      break;
    default:
      if (s_warning_type != WARNING_TYPE_INPUT) break;
      s_editMode = EDIT_MODIFY_FIELD;
      s_warning_input_value = checkIncDec(event, s_warning_input_value, s_warning_input_min, s_warning_input_max);
      s_editMode = EDIT_SELECT_FIELD;
      break;
  }
}

void (*popupFunc)(evt_t event) = NULL;

const char *s_menu[MENU_MAX_LINES];
uint8_t s_menu_item = 0;
uint16_t s_menu_count = 0;
uint8_t s_menu_flags = 0;
uint16_t s_menu_offset = 0;
void (*menuHandler)(const char *result);
const char * displayMenu(evt_t event)
{
  const char * result = NULL;
  uint8_t display_count = min(s_menu_count, (uint16_t)MENU_MAX_LINES);

  switch (event) {
    case EVT_ROTARY_LEFT:
    case EVT_KEY_FIRST(KEY_UP):
    case EVT_KEY_REPT(KEY_UP):
      if (s_menu_item > 0) {
        s_menu_item--;
      }
      else if (s_menu_offset > 0) {
        s_menu_offset--;
        result = STR_UPDATE_LIST;
      }
      else {
        s_menu_item = display_count - 1;
        if (s_menu_count > MENU_MAX_LINES) {
          s_menu_offset = s_menu_count - display_count;
          result = STR_UPDATE_LIST;
        }
      }
      break;

    case EVT_ROTARY_RIGHT:
    case EVT_KEY_FIRST(KEY_DOWN):
    case EVT_KEY_REPT(KEY_DOWN):
      if (s_menu_item < display_count - 1 && s_menu_offset + s_menu_item + 1 < s_menu_count) {
        s_menu_item++;
      }
      else if (s_menu_count > s_menu_offset + display_count) {
        s_menu_offset++;
        result = STR_UPDATE_LIST;
      }
      else {
        s_menu_item = 0;
        if (s_menu_offset) {
          s_menu_offset = 0;
          result = STR_UPDATE_LIST;
        }
      }
      break;
    case EVT_KEY_BREAK(KEY_ENTER):
      result = s_menu[s_menu_item];
      // no break
    case EVT_KEY_BREAK(KEY_EXIT):
      s_menu_count = 0;
      s_menu_item = 0;
      s_menu_flags = 0;
      s_menu_offset = 0;
      break;
  }

  int y = (LCD_H - (display_count*(FH+1))) / 2;

  lcdDrawSolidFilledRect(MENU_X, y, MENU_W, display_count * (FH+1), TEXT_BGCOLOR);
  lcdDrawSolidRect(MENU_X, y, MENU_W, display_count * (FH+1) + 1, ALARM_COLOR);

  for (uint8_t i=0; i<display_count; i++) {
    if (i == s_menu_item) {
      lcdDrawSolidFilledRect(MENU_X+1, i*(FH+1) + y + 1, MENU_W-2, FH+1, TEXT_INVERTED_BGCOLOR);
      lcdDrawText(MENU_X+6, i*(FH+1) + y + 4, s_menu[i], TEXT_INVERTED_COLOR|s_menu_flags);
    }
    else {
      lcdDrawText(MENU_X+6, i*(FH+1) + y + 4, s_menu[i], s_menu_flags);
    }
  }

  if (s_menu_count > display_count) {
    drawVerticalScrollbar(MENU_X+MENU_W-1, y+1, MENU_MAX_LINES * (FH+1), s_menu_offset, s_menu_count, MENU_MAX_LINES);
  }

  return result;
}