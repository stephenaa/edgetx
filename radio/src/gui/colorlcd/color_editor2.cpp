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

constexpr int COLOR_BOX_TOP = 5;
constexpr int COLOR_BOX_HEIGHT = 32;

constexpr int BUTTON_LEFT = 3;
constexpr int BUTTON_TOP = COLOR_BOX_TOP + COLOR_BOX_HEIGHT + 15;
constexpr int BUTTON_HEIGHT = 20;
constexpr int BUTTON_WIDTH = 50;
constexpr int BUTTON_MARGIN = 4;

constexpr int BAR_LEFT = BUTTON_LEFT + BUTTON_WIDTH + BUTTON_MARGIN;
constexpr int BAR_WIDTH = 20;
constexpr int BAR_MARGIN = 2;

constexpr int BAR_HEIGHT = 160;
constexpr int BAR_TOP_MARGIN = 50;

constexpr int COLOR_BOX_WIDTH = BUTTON_WIDTH;
constexpr int COLOR_BOX_LEFT = 3;

const char *RGBChars[MAX_BARS] = { "R", "G", "B" };
const char *HSVChars[MAX_BARS] = { "H", "S", "V" };

HSVColorType::HSVColorType(FormGroup *window, uint32_t color)
{
  screenHeight = BAR_HEIGHT;
  auto r = GET_RED(color), g = GET_GREEN(color), b = GET_BLUE(color);
  float values[MAX_BARS];
  RGBtoHSV(r, g, b, values[0], values[1], values[2]);
  values[1] *= MAX_SATURATION; // convert the proper base
  values[2] *= MAX_BRIGHTNESS;

  for (auto i = 0; i < MAX_BARS; i++) {
    auto left = BAR_LEFT + ((BAR_WIDTH + BAR_MARGIN) * i);
    barInfo[i].leftPos = left;
    barInfo[i].maxValue = i == 0 ? MAX_HUE : MAX_BRIGHTNESS;
    barInfo[i].invert = i != 0;
    barInfo[i].sliding = false;
    barInfo[i].value = values[i];
    barInfo[i].barText = 
      new StaticText(window, { left, BAR_TOP_MARGIN - 20, BAR_WIDTH, 16 },
                     std::to_string(barInfo[i].value), 
                     0, COLOR_THEME_PRIMARY1 | CENTERED | FONT(XS));
  }
}

uint32_t HSVColorType::getBarValue(int bar, coord_t pos)
{
  auto value = screenToValue(bar, pos);
  value = max(value, (uint32_t)0);
  value = min(value, barInfo[bar].maxValue);
  return value;
}

uint32_t HSVColorType::getRGB()
{
  return HSVtoRGB(barInfo[0].value, barInfo[1].value, barInfo[2].value);
}

void HSVColorType::drawBar(BitmapBuffer* dc, int bar, getRGBFromPos getRGB)
{
  int maxRange = BAR_HEIGHT;
  for (auto i = 0; i < maxRange; i++) {
    auto rgb = getRGB(bar, i);
    auto r = GET_RED(rgb);
    auto g = GET_GREEN(rgb);
    auto b = GET_BLUE(rgb);

    dc->drawSolidHorizontalLine(barInfo[bar].leftPos, i + BAR_TOP_MARGIN, BAR_WIDTH, COLOR2FLAGS(RGB(r, g, b)));
  }

  dc->drawSolidRect(barInfo[bar].leftPos, BAR_TOP_MARGIN, BAR_WIDTH, screenHeight, 1, COLOR2FLAGS(BLACK));
  dc->drawFilledCircle(barInfo[bar].leftPos + (BAR_WIDTH / 2), BAR_TOP_MARGIN + valueToScreen(bar, barInfo[bar].value), 5, COLOR2FLAGS(WHITE));
  dc->drawText(barInfo[bar].leftPos + 3, BAR_TOP_MARGIN + screenHeight + 2, HSVChars[bar], COLOR_THEME_PRIMARY1 | FONT(XS));
}

void HSVColorType::paint(BitmapBuffer *dc)
{
  drawBar(dc, 0, [=] (int bar, int pos) {
    int hue = screenToValue(0, pos);
    auto rgb = HSVtoRGB(hue, MAX_SATURATION, MAX_BRIGHTNESS);
    return rgb;
  });
  drawBar(dc, 1, [=] (int bar, int pos) {
    int saturation = screenToValue(1, pos);
    auto rgb = HSVtoRGB(barInfo[0].value, saturation, 50);
    return rgb;
  });
  drawBar(dc, 2, [=] (int bar, int pos) {
    int brightness = screenToValue(2, pos);
    auto rgb = HSVtoRGB(barInfo[0].value, 100, brightness);
    return rgb;
  });
}

uint32_t RGBColorType::getBarValue(int bar, coord_t pos)
{
  uint32_t value = ((float)pos / BAR_HEIGHT) * barInfo[bar].maxValue;
  value = max(value, (uint32_t)0);
  value = min(value, barInfo[bar].maxValue);
  return value;
}

uint32_t RGBColorType::getRGB()
{
  return RGB(barInfo[0].value, barInfo[1].value, barInfo[2].value);
}

void RGBColorType::paint(BitmapBuffer *dc)
{
  uint32_t maxRange = BAR_HEIGHT;
  for (int i = 0; i < MAX_BARS; i++) {
    for (uint32_t j = 0; j < maxRange; j++) {
      int value = (((float)j) / maxRange) * barInfo[i].maxValue;
      uint32_t color;
      if (i == 0)
        color = RGB(value, 0, 0);
      else if (i == 1)
        color = RGB(0, value, 0);
      else
        color = RGB(0, 0, value);

      dc->drawSolidHorizontalLine(barInfo[i].leftPos, j + BAR_TOP_MARGIN, BAR_WIDTH, 
                                  COLOR2FLAGS(color));
      dc->drawSolidRect(barInfo[i].leftPos, BAR_TOP_MARGIN, BAR_WIDTH, maxRange, 1, 
                        COLOR2FLAGS(BLACK));
    }

    auto scalledValue = ((float)barInfo[i].value / barInfo[i].maxValue) * maxRange;
    dc->drawFilledCircle(barInfo[i].leftPos + (BAR_WIDTH / 2), BAR_TOP_MARGIN + scalledValue, 5, 
                        COLOR2FLAGS(WHITE));
    dc->drawText(barInfo[i].leftPos + 3, BAR_TOP_MARGIN + maxRange + 2, RGBChars[i], 
                 COLOR_THEME_PRIMARY1 | FONT(XS));
  }
}

RGBColorType::RGBColorType(FormGroup *window, uint32_t color)
{
  screenHeight = BAR_HEIGHT;
  auto r = GET_RED(color), g = GET_GREEN(color), b = GET_BLUE(color);
  float values[MAX_BARS];
  values[0] = r; values[1] = g; values[2] = b;

  for (auto i = 0; i < MAX_BARS; i++) {
    auto left = BAR_LEFT + ((BAR_WIDTH + BAR_MARGIN) * i);
    barInfo[i].leftPos = left;
    barInfo[i].maxValue = 255;
    barInfo[i].sliding = false;
    barInfo[i].value = values[i];
    barInfo[i].barText = new StaticText(window, { left, BAR_TOP_MARGIN - 20, BAR_WIDTH, 16 },
      std::to_string(barInfo[i].value), 0, COLOR_THEME_PRIMARY1 | CENTERED | FONT(XS));
  }
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
void setHexStr(StaticText* hexBox, uint32_t rgb)
{
  auto r = GET_RED(rgb), g = GET_GREEN(rgb), b = GET_BLUE(rgb);
  char hexstr[80];
  sprintf(hexstr, "%02X%02X%02X\n", r, g, b);
  hexBox->setText(hexstr);
}

ColorEditor::ColorEditor(FormGroup *window, const rect_t rect, uint32_t color,
                         std::function<void (uint32_t rgb)> setValue) :
  FormGroup(window, rect),
  _setValue(std::move(setValue)),
  _color(color)
{
  colorType = new HSVColorType(this, color);
  
  hexBox = new StaticText(this, { BAR_LEFT, COLOR_BOX_TOP, 80, 20 }, 
                          "", 0, COLOR_THEME_PRIMARY1);
  setHexStr(hexBox, _color);

  rect_t a = { BUTTON_LEFT, BUTTON_TOP, BUTTON_WIDTH, BUTTON_HEIGHT };
  firstButton = new TextButton(this, a, "HSV", 
    [=] () {
      if (colorType != nullptr) {
        delete colorType;
      }
      colorType = new HSVColorType(this, color);
      invalidate();
      return 0;
    }, 0, COLOR_THEME_PRIMARY1);
  a.y = a.y + BUTTON_HEIGHT + BUTTON_MARGIN;
  lastButton = new TextButton(this, a, "RGB", 
    [=] () {
      if (colorType != nullptr) {
        delete colorType;
      }
      colorType = new RGBColorType(this, color);
      invalidate();
      return 0;
    }, 0, COLOR_THEME_PRIMARY1);

  setFocusHandler([=](bool focus) {
    focused = focus;
  });

  
  firstButton->setPreviousField(this);
  lastButton->setNextField(this);
  firstButton->setFocus();
}

void ColorEditor::setRGB()
{
  _color = colorType->getRGB();
  for (auto i = 0; i < MAX_BARS; i++) {
    if (colorType->barInfo[i].barText != nullptr) {
      colorType->barInfo[i].barText->setText(std::to_string(colorType->barInfo[i].value));
    }
  }
  setHexStr(hexBox, _color);

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
    if (colorType->barInfo[i].sliding) {
      y -= BAR_TOP_MARGIN;
      auto value = colorType->getBarValue(i, y);
      if (value != colorType->barInfo[i].value) {
        slidingWindow = this;
        colorType->barInfo[i].value = value;    
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
    if (colorType->barInfo[i].sliding) {
      bSliding = true;
    }
    colorType->barInfo[i].sliding = false;
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
        x >= colorType->barInfo[i].leftPos && x < colorType->barInfo[i].leftPos + BAR_WIDTH) {
      colorType->barInfo[i].sliding = true;
      y -= BAR_TOP_MARGIN;
      int value = colorType->getBarValue(i, y);
      colorType->barInfo[i].value = value;
      bFound = true;
    } else {
      colorType->barInfo[i].sliding = false;
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
          auto bar = &colorType->barInfo[focusBar];
          bar->value += direction; 
          bar->value = min(bar->value, bar->maxValue);
          bar->value = max(bar->value, (uint32_t)0);
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

void ColorEditor::drawColorBox(BitmapBuffer *dc)
{
  dc->drawSolidFilledRect(COLOR_BOX_LEFT, COLOR_BOX_TOP, COLOR_BOX_WIDTH, 
                COLOR_BOX_HEIGHT, COLOR2FLAGS(_color));
  dc->drawSolidRect(COLOR_BOX_LEFT, COLOR_BOX_TOP, COLOR_BOX_WIDTH, 
                COLOR_BOX_HEIGHT, 1, COLOR2FLAGS(BLACK));
}

void ColorEditor::drawFocusBox(BitmapBuffer *dc)
{
  if (focused && !isEditMode()) {
    auto bar = colorType->barInfo[focusBar];
    dc->drawSolidRect(bar.leftPos, BAR_TOP_MARGIN, BAR_WIDTH, BAR_HEIGHT, 2, COLOR_THEME_FOCUS);
  } else if (isEditMode()) {
    auto bar = colorType->barInfo[focusBar];
    dc->drawSolidRect(bar.leftPos, BAR_TOP_MARGIN, BAR_WIDTH, BAR_HEIGHT, 1, COLOR_THEME_EDIT);
  }
}

void ColorEditor::paint(BitmapBuffer *dc)
{
  dc->clear(COLOR_THEME_SECONDARY3);
  dc->drawSolidRect(0, 0, rect.w, rect.h, 1, COLOR2FLAGS(BLACK));
  colorType->paint(dc);
  drawColorBox(dc);
  drawFocusBox(dc);
}

