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
#include "color_editor2.h"

#define SET_DIRTY() setDirty()

////////////////////////////////////////////////////////////////////////////////////////////////
/////                     ColorEditorContent
////////////////////////////////////////////////////////////////////////////////////////////////
constexpr int MAX_SATURATION         = 100;
constexpr int MAX_HUE                = 360;
constexpr int BAR_HEIGHT             = MAX_HUE / 2;
    
constexpr int BAR_TOP_MARGIN         = 27;
     
constexpr int COLOR_BOX_TOP          = 130;
constexpr int COLOR_BOX_WIDTH        = 90;
constexpr int COLOR_BOX_HEIGHT       = 32;

constexpr int COLOR_BOX_LEFT         = 240;


ColorEditor::ColorEditor(FormGroup *window, const rect_t rect, uint32_t color,
                         std::function<void (uint32_t rgb)> setValue) :
  FormGroup(window, rect),
  _setValue(std::move(setValue)),
  _color(color)
{
  Layer::push(this);
  auto r = GET_RED(_color), g = GET_GREEN(_color), b = GET_BLUE(_color);
  float values[MAX_BARS];
  RGBtoHSV(r, g, b, values[0], values[1], values[2]);

  for (auto i = 0; i < MAX_BARS; i++) {
    auto left = BAR_LEFT + ((BAR_WIDTH + BAR_MARGIN) * i);
    barInfo[i].barText = new StaticText(this, {left, BAR_TOP_MARGIN - 20, BAR_WIDTH, 16 }, 
                                std::to_string(i * 23), 0, COLOR_THEME_PRIMARY1 | CENTERED | FONT(XS));
    barInfo[i].leftPos = left;
    barInfo[i].maxValue = i == 0 ? MAX_HUE : MAX_BRIGHTNESS;
    barInfo[i].sliding = false;
    barInfo[i].value = values[i];
  }

  firstButton = new TextButton(this, { 3, 40, 40, 20 }, "PAL", 0, COLOR_THEME_PRIMARY1);
  new TextButton(this, { 3, 64, 40, 20 }, "RGB", 0, COLOR_THEME_PRIMARY1);
  lastButton = new TextButton(this, { 3, 88, 40, 20 }, "HSV", 0, COLOR_THEME_PRIMARY1);
  firstButton->setFocus();
  firstButton->setPreviousField(this);
  lastButton->setNextField(this);

  setFocusHandler([=] (bool focus) {
    focused = focus;
  });
}

void ColorEditor::setRGB()
{
  for (auto i = 0; i < MAX_BARS; i++) {
    barInfo[i].barText->setText(std::to_string(barInfo[i].value));
  }

  invalidate();
  if (_setValue != nullptr)
    _setValue(_color);
}

#if defined(HARDWARE_TOUCH)
bool ColorEditor::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  if (touchState.event == TE_SLIDE_END) {
    onTouchEnd(0,0);
    return true;
  }

  for (auto i = 0; i < MAX_BARS; i++) {
    if (barInfo[i].sliding) {
      y -= BAR_TOP_MARGIN;
      y = max(y, 0);
      y = i != 0 ? (((float)y / BAR_HEIGHT) * 100) : y * 2;
      y = min(y, barInfo[i].maxValue);
      if (y != barInfo[i].value) {
        barInfo[i].value = y;    
        setRGB(); 
      } 
    }
  }

  return true;
}

bool ColorEditor::onTouchEnd(coord_t x, coord_t y)
{
  bool bSliding = false;
  for (auto i = 0; i < MAX_BARS; i++) {
    if (barInfo[i].sliding) {
      bSliding = true;
    }
    barInfo[i].sliding = false;
  }

  if (bSliding) {
    invalidate();
  }

  return FormGroup::onTouchEnd(x,y);
}

bool ColorEditor::onTouchStart(coord_t x, coord_t y)
{
  bool bFound = false;
  for (auto i = 0; i < MAX_BARS; i++) {
    if (y >= BAR_TOP_MARGIN && y < BAR_TOP_MARGIN + BAR_HEIGHT &&
        x >= barInfo[i].leftPos && x < barInfo[i].leftPos + BAR_WIDTH) {
      barInfo[i].sliding = true;
      y -= BAR_TOP_MARGIN;
      int value = 
        i == 0 ? 
            y * 2 : 
            (((float)y) / BAR_HEIGHT) * 100;
        barInfo[i].value = value;
        bFound = true;
    } else {
      barInfo[i].sliding = false;
    }
  }
  if (bFound) {
    setRGB();
    return true;
  } else {
    return FormGroup::onTouchStart(x,y);
  }
}
#endif

#if defined(HARDWARE_KEYS)
void ColorEditor::onEvent(event_t event)
{
  switch(event)
  {
    case EVT_KEY_BREAK(KEY_ENTER):
      if (focused) {
        setEditMode(!isEditMode());
      }
      break;
    case EVT_ROTARY_RIGHT:
    case EVT_ROTARY_LEFT:
      {
        int direction = event == EVT_ROTARY_RIGHT ? 1 : -1;
        if (isEditMode()) {
          auto bar = &barInfo[focusBar];
          bar->value += direction; 
          bar->value = min(bar->value, bar->maxValue);
          bar->value = max(bar->value, 0);
          setRGB();
        }
        else if (focused) {
          focusBar += direction;
          if (focusBar == MAX_BARS) {
            focusBar = 0;
            firstButton->setFocus();
          } else if (focusBar < 0) {
            focusBar = 0;
            lastButton->setFocus();
          }
          invalidate();
        }
      }
    break;
    default:
      FormGroup::onEvent(event);
      break;
  }
}
#endif


void ColorEditor::drawHueBar(BitmapBuffer *dc)
{
  for (int i = 0; i < MAX_HUE; i+= 2) {
    auto rgb = HSVtoRGB(i, MAX_SATURATION, MAX_BRIGHTNESS);
    auto r = GET_RED(rgb);
    auto g = GET_GREEN(rgb);
    auto b = GET_BLUE(rgb);
    dc->drawSolidHorizontalLine(barInfo[0].leftPos, BAR_TOP_MARGIN + i/2, BAR_WIDTH, COLOR2FLAGS(RGB(r, g, b)));
  }

  dc->drawSolidRect(barInfo[0].leftPos, BAR_TOP_MARGIN, BAR_WIDTH, MAX_HUE / 2, 1, COLOR2FLAGS(BLACK));
  dc->drawFilledCircle(barInfo[0].leftPos + (BAR_WIDTH / 2), BAR_TOP_MARGIN + barInfo[0].value / 2, 5, COLOR2FLAGS(WHITE));
 
  dc->drawText(barInfo[0].leftPos + 3, BAR_TOP_MARGIN + MAX_HUE / 2 + 2, "H", COLOR_THEME_PRIMARY1 |FONT(XS));
}

void ColorEditor::drawBrightnessBar(BitmapBuffer *dc) 
{
  int maxRange = BAR_HEIGHT;
  for (auto i = 0; i < maxRange; i++) {
    int brightness = (((float)i / maxRange) * 100);
    auto rgb = HSVtoRGB(barInfo[0].value, 100, brightness);
    auto r = GET_RED(rgb);
    auto g = GET_GREEN(rgb);
    auto b = GET_BLUE(rgb);

    dc->drawSolidHorizontalLine(barInfo[2].leftPos, BAR_HEIGHT - i + BAR_TOP_MARGIN, BAR_WIDTH, COLOR2FLAGS(RGB(r, g, b)));
    dc->drawSolidRect(barInfo[2].leftPos, BAR_TOP_MARGIN, BAR_WIDTH, MAX_HUE / 2, 1, COLOR2FLAGS(BLACK));
  }

  auto scalledValue = ((float)barInfo[2].value / 100) * maxRange;
  dc->drawFilledCircle(barInfo[2].leftPos + (BAR_WIDTH / 2), BAR_TOP_MARGIN + scalledValue, 5, COLOR2FLAGS(WHITE));
  dc->drawText(barInfo[2].leftPos + 3, BAR_TOP_MARGIN + maxRange + 2, "B", COLOR_THEME_PRIMARY1 | FONT(XS));
}

void ColorEditor::drawSaturationBar(BitmapBuffer *dc)
{
  int maxRange = BAR_HEIGHT;
  for (auto i = 0; i < maxRange; i++) {
    int saturation = (((float)i / maxRange) * 100);
    auto rgb = HSVtoRGB(barInfo[0].value, saturation, 50);
    auto r = GET_RED(rgb);
    auto g = GET_GREEN(rgb);
    auto b = GET_BLUE(rgb);

    dc->drawSolidHorizontalLine(barInfo[1].leftPos, BAR_HEIGHT - i + BAR_TOP_MARGIN, BAR_WIDTH, COLOR2FLAGS(RGB(r, g, b)));
    dc->drawSolidRect(barInfo[1].leftPos, BAR_TOP_MARGIN, BAR_WIDTH, MAX_HUE/2, 1, COLOR2FLAGS(BLACK));
  }

  auto scalledValue = ((float)barInfo[1].value / 100) * maxRange;
  dc->drawFilledCircle(barInfo[1].leftPos + (BAR_WIDTH / 2), BAR_TOP_MARGIN + scalledValue, 5, COLOR2FLAGS(WHITE));
  dc->drawText(barInfo[1].leftPos + 3, BAR_TOP_MARGIN + maxRange + 2, "S", COLOR_THEME_PRIMARY1 | FONT(XS));
}

void ColorEditor::drawColorBox(BitmapBuffer *dc)
{
}

void ColorEditor::drawFocusBox(BitmapBuffer *dc)
{
  if (focused && !isEditMode()) {
    auto bar = barInfo[focusBar];
    dc->drawSolidRect(bar.leftPos, BAR_TOP_MARGIN, BAR_WIDTH, BAR_HEIGHT, 2, COLOR_THEME_FOCUS);
  } else if (isEditMode()) {
    auto bar = barInfo[focusBar];
    dc->drawSolidRect(bar.leftPos, BAR_TOP_MARGIN, BAR_WIDTH, BAR_HEIGHT, 1, COLOR_THEME_EDIT);
  }
}

void ColorEditor::paint(BitmapBuffer *dc)
{
  dc->clear(COLOR_THEME_SECONDARY3);
  dc->drawSolidRect(0, 0, rect.w, rect.h, 1, COLOR2FLAGS(BLACK));
  drawHueBar(dc);
  drawSaturationBar(dc);
  drawBrightnessBar(dc);
  drawColorBox(dc);
  drawFocusBox(dc);
}

