/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#pragma once
#include <algorithm>
#include <vector>
#include <iostream>
#include "bitmapbuffer.h"
#include "libopenui.h"
#include "theme_manager.h"
#include "listbox.h"

// the content page of the ColorEditorPupup
class ColorEditorContent : public ModalWindowContent
{
  friend class ColorEditorPopup;
  public:
    ColorEditorContent(ModalWindow *window, const rect_t rect, uint32_t color, std::function<void (uint32_t rgb)> setValue = nullptr);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ColorEditorContent";
    }
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY) override;
    bool onTouchEnd(coord_t x, coord_t y) override;
    bool onTouchStart(coord_t x, coord_t y) override;
#endif

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

    void paint(BitmapBuffer *dc) override;

  protected:
    bool sliding = false;
    bool colorPicking = false;
    int r, g, b;
    int hue = 0, s, v;
    NumberEdit *rEdit;
    NumberEdit *gEdit;
    NumberEdit *bEdit;

    void drawHueBar(BitmapBuffer *dc);
    void drawGrid(BitmapBuffer *dc);
    void drawColorBox(BitmapBuffer *dc);
    void setRGB();
    std::function<void (uint32_t rgb)> setValue;
};

// a color editor popup
class ColorEditorPopup : public ModalWindow
{
  public:
    ColorEditorPopup(Window *window, std::function<uint32_t ()> getValue, std::function<void (uint32_t value)> setValue = nullptr);
    
    void deleteLater(bool detach = true, bool trash = true) override;

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ColorEditorPopup";
    }
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchStart(coord_t x, coord_t y) override;
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

  protected:
    uint32_t color;
    ColorEditorContent *content;
    std::function<uint32_t ()> _getValue;
    std::function<void (uint32_t value)> _setValue;
};

class ColorSquare : public FormField
{
  public:
    ColorSquare(Window *window, const rect_t &rect, std::function<void (ColorEntry value)> setValue = nullptr)
      : FormField(window, rect),
      setValue(setValue)
    {
    }

#define PREVIEW_Y_OFFSET 5
#define PREVIEW_HEIGHT 22
  
    void paint (BitmapBuffer *dc) override
    {
      if (colorEntry.colorNumber != COLOR_COUNT)
        dc->drawSolidFilledRect(0, 0, rect.w, rect.h, COLOR2FLAGS(RGB(r, g, b)));

      LcdFlags flags = hasFocus() ? COLOR_THEME_FOCUS : COLOR2FLAGS(BLACK);
      int width = hasFocus() ? 2 : 1;
      dc->drawSolidRect(0, 0, rect.w, rect.h, width, flags);
    }

    void setColorToEdit(ColorEntry colorEntry)
    {
      this->colorEntry = colorEntry;
      r = GET_RED(colorEntry.colorValue);
      g = GET_GREEN(colorEntry.colorValue);
      b = GET_BLUE(colorEntry.colorValue);

      invalidate();
    }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override
  {
    if (event == EVT_KEY_BREAK(KEY_ENTER)) {
      new ColorEditorPopup(this, 
        [=] () { 
          return RGB( r, g, b ); 
        }, 
        [=] (uint32_t rgb) {
          r = GET_RED(rgb);
          g = GET_GREEN(rgb);
          b = GET_BLUE(rgb);
          colorEntry.colorValue = rgb;
          setValue(colorEntry);
        });
    } else {
      FormField::onEvent(event);
    }
  }
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchStart(coord_t x, coord_t y) override
    {
      new ColorEditorPopup(this, 
        [=] () { 
          return RGB( r, g, b ); 
        }, 
        [=] (uint32_t rgb) {
          r = GET_RED(rgb);
          g = GET_GREEN(rgb);
          b = GET_BLUE(rgb);
          colorEntry.colorValue = rgb;
          setValue(colorEntry);
        });
      return true;
    }
#endif

  protected:
    ColorEntry colorEntry = {LCD_COLOR_COUNT, 0};
    uint32_t r = 0;
    uint32_t g = 0;
    uint32_t b = 0;
    std::function<void (ColorEntry colorEntry)> setValue;
};


