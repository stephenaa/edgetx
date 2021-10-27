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

constexpr int MAX_BARS = 3;
class Bar
{
  public:
    bool sliding;
    int leftPos;
    int maxValue;
    int value;
    StaticText* barText;
};

class ColorType
{
  public:
    ColorType()
    {
      for (auto i = 0; i < MAX_BARS; i++) {
        barInfo[i].barText = nullptr;
      }
    }
    virtual ~ColorType()
    {
      for (auto i = 0; i < MAX_BARS; i++) {
        if (barInfo[i].barText != nullptr) {
          barInfo[i].barText->deleteLater();
        }
      }
    };
    Bar barInfo[MAX_BARS];
    virtual void paint(BitmapBuffer *dc) {}
    virtual uint32_t getBarValue(int bar, coord_t pos) {return 0;}
    virtual uint32_t getRGB() {return 0;}
};

class HSVColorType : public ColorType
{
  public:
    HSVColorType(FormGroup *window, uint32_t color);
    uint32_t getBarValue(int bar, coord_t pos) override;
    uint32_t getRGB() override;

    void drawHueBar(BitmapBuffer* dc);
    void drawSaturationBar(BitmapBuffer* dc);
    void drawBrightnessBar(BitmapBuffer* dc);
    void paint(BitmapBuffer* dc) override;
};

class RGBColorType : public ColorType
{
  public:

    RGBColorType(FormGroup *window, uint32_t color);
    uint32_t getBarValue(int bar, coord_t pos) override;
    uint32_t getRGB() override;

    void paint(BitmapBuffer* dc) override;
};

class PalletColorType : public ColorType
{
public:

  PalletColorType(FormGroup* window, uint32_t color);
  ~PalletColorType() override;
  uint32_t getBarValue(int bar, coord_t pos) override;
  uint32_t getRGB() override;

  void paint(BitmapBuffer* dc) override;
};


// the content page of the ColorEditorPupup
class ColorEditor : public FormGroup
{
  public:
    ColorEditor(FormGroup *window, const rect_t rect, uint32_t color, std::function<void (uint32_t rgb)> setValue = nullptr);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ColorEditor";
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
    ColorType *colorType = nullptr;
    TextButton *firstButton = nullptr, *lastButton = nullptr;
    void drawColorBox(BitmapBuffer *dc);
    void drawFocusBox(BitmapBuffer *dc);
    void setRGB();
    std::function<void (uint32_t rgb)> _setValue;
    uint32_t _color;
    int focusBar = 0;
    bool focused = false;
};

