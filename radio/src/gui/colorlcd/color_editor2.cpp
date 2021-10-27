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

constexpr int BUTTON_LEFT = 3;
constexpr int BUTTON_TOP = 80;
constexpr int BUTTON_HEIGHT = 20;
constexpr int BUTTON_WIDTH = 50;
constexpr int BUTTON_MARGIN = 4;

constexpr int BAR_LEFT = BUTTON_LEFT + BUTTON_WIDTH + BUTTON_MARGIN;
constexpr int BAR_WIDTH = 20;
constexpr int BAR_MARGIN = 2;

constexpr int BAR_HEIGHT = MAX_HUE / 2;
constexpr int BAR_TOP_MARGIN = 27;

constexpr int COLOR_BOX_TOP = BAR_TOP_MARGIN;
constexpr int COLOR_BOX_WIDTH = BUTTON_WIDTH;
constexpr int COLOR_BOX_HEIGHT = 32;
constexpr int COLOR_BOX_LEFT = 3;

HSVColorType::HSVColorType(FormGroup *window, uint32_t color) 
{
  auto r = GET_RED(color), g = GET_GREEN(color), b = GET_BLUE(color);
  float values[MAX_BARS];
  RGBtoHSV(r, g, b, values[0], values[1], values[2]);
  values[1] *= MAX_SATURATION; // convert the proper base
  values[2] *= MAX_BRIGHTNESS;

  for (auto i = 0; i < MAX_BARS; i++) {
    auto left = BAR_LEFT + ((BAR_WIDTH + BAR_MARGIN) * i);
    barInfo[i].leftPos = left;
    barInfo[i].maxValue = i == 0 ? MAX_HUE : MAX_BRIGHTNESS;
    barInfo[i].sliding = false;
    barInfo[i].value = values[i];
    barInfo[i].barText = new StaticText(window, { left, BAR_TOP_MARGIN - 20, BAR_WIDTH, 16 },
      std::to_string(barInfo[i].value), 0, COLOR_THEME_PRIMARY1 | CENTERED | FONT(XS));
  }
}

uint32_t HSVColorType::getBarValue(int bar, coord_t pos)
{
  int value = bar != 0 ? 100 - (((float)pos / BAR_HEIGHT) * 100) : pos * 2;
  value = max(value, 0);
  value = min(value, barInfo[bar].maxValue);
  return value;
}

uint32_t HSVColorType::getRGB()
{
  return HSVtoRGB(barInfo[0].value, barInfo[1].value, barInfo[2].value);
}

void HSVColorType::drawHueBar(BitmapBuffer* dc)
{
  for (int i = 0; i < MAX_HUE; i += 2) {
    auto rgb = HSVtoRGB(i, MAX_SATURATION, MAX_BRIGHTNESS);
    auto r = GET_RED(rgb);
    auto g = GET_GREEN(rgb);
    auto b = GET_BLUE(rgb);
    dc->drawSolidHorizontalLine(barInfo[0].leftPos, BAR_TOP_MARGIN + i / 2, BAR_WIDTH, COLOR2FLAGS(RGB(r, g, b)));
  }

  dc->drawSolidRect(barInfo[0].leftPos, BAR_TOP_MARGIN, BAR_WIDTH, MAX_HUE / 2, 1, COLOR2FLAGS(BLACK));
  dc->drawFilledCircle(barInfo[0].leftPos + (BAR_WIDTH / 2), BAR_TOP_MARGIN + barInfo[0].value / 2, 5, COLOR2FLAGS(WHITE));

  dc->drawText(barInfo[0].leftPos + 3, BAR_TOP_MARGIN + MAX_HUE / 2 + 2, "H", COLOR_THEME_PRIMARY1 | FONT(XS));
}

void HSVColorType::drawBrightnessBar(BitmapBuffer* dc)
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
  dc->drawFilledCircle(barInfo[2].leftPos + (BAR_WIDTH / 2), BAR_TOP_MARGIN + (maxRange - scalledValue), 5, COLOR2FLAGS(WHITE));
  dc->drawText(barInfo[2].leftPos + 3, BAR_TOP_MARGIN + maxRange + 2, "B", COLOR_THEME_PRIMARY1 | FONT(XS));
}

void HSVColorType::drawSaturationBar(BitmapBuffer* dc)
{
  int maxRange = BAR_HEIGHT;
  for (auto i = 0; i < maxRange; i++) {
    int saturation = (((float)i / maxRange) * 100);
    auto rgb = HSVtoRGB(barInfo[0].value, saturation, 50);
    auto r = GET_RED(rgb);
    auto g = GET_GREEN(rgb);
    auto b = GET_BLUE(rgb);

    dc->drawSolidHorizontalLine(barInfo[1].leftPos, BAR_HEIGHT - i + BAR_TOP_MARGIN, BAR_WIDTH, COLOR2FLAGS(RGB(r, g, b)));
    dc->drawSolidRect(barInfo[1].leftPos, BAR_TOP_MARGIN, BAR_WIDTH, MAX_HUE / 2, 1, COLOR2FLAGS(BLACK));
  }

  auto scalledValue = ((float)barInfo[1].value / 100) * maxRange;
  dc->drawFilledCircle(barInfo[1].leftPos + (BAR_WIDTH / 2), BAR_TOP_MARGIN + (maxRange - scalledValue), 5, COLOR2FLAGS(WHITE));
  dc->drawText(barInfo[1].leftPos + 3, BAR_TOP_MARGIN + maxRange + 2, "S", COLOR_THEME_PRIMARY1 | FONT(XS));
}

void HSVColorType::paint(BitmapBuffer *dc)
{
  drawHueBar(dc);
  drawSaturationBar(dc);
  drawBrightnessBar(dc);
}

uint32_t RGBColorType::getBarValue(int bar, coord_t pos)
{
  int value =((float)pos / BAR_HEIGHT) * barInfo[bar].maxValue;
  value = max(value, 0);
  value = min(value, barInfo[bar].maxValue);
  return value;
}

uint32_t RGBColorType::getRGB()
{
  return RGB(barInfo[0].value, barInfo[1].value, barInfo[2].value);
}

std::string RGBChars[3] = {"R", "G", "B"};

void RGBColorType::paint(BitmapBuffer *dc)
{
  int maxRange = BAR_HEIGHT;
  for (int i = 0; i < MAX_BARS; i++) {
    for (uint32_t j = 0; j < maxRange; j++) {
      int value = (((float)j) / maxRange) * 255;
      uint32_t color;
      if (i == 0)
        color = RGB(value, 0, 0);
      else if (i == 1)
        color = RGB(0, value, 0);
      else
        color = RGB(0, 0, value);

      dc->drawSolidHorizontalLine(barInfo[i].leftPos, j + BAR_TOP_MARGIN, BAR_WIDTH, COLOR2FLAGS(color));
      dc->drawSolidRect(barInfo[i].leftPos, BAR_TOP_MARGIN, BAR_WIDTH, maxRange, 1, COLOR2FLAGS(BLACK));
    }

    auto scalledValue = ((float)barInfo[i].value / 255) * maxRange;
    dc->drawFilledCircle(barInfo[i].leftPos + (BAR_WIDTH / 2), BAR_TOP_MARGIN + scalledValue, 5, COLOR2FLAGS(WHITE));
    dc->drawText(barInfo[i].leftPos + 3, BAR_TOP_MARGIN + maxRange + 2, RGBChars[i].c_str(), COLOR_THEME_PRIMARY1 | FONT(XS));
  }
}

RGBColorType::RGBColorType(FormGroup *window, uint32_t color)
{
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


PalletColorType::PalletColorType(FormGroup *window, uint32_t color)
{
  for (auto i = 0; i < MAX_BARS; i++) {
    barInfo[i].barText = nullptr;
  }
}

PalletColorType::~PalletColorType()
{
}

void PalletColorType::paint(BitmapBuffer *dc)
{
  for (auto i = 0; i < 8; i++) {
    for (auto j = 0; j < 15; j++) {
      dc->drawSolidFilledRect(BAR_LEFT + (i * 11), BAR_TOP_MARGIN + (j * 11), 11, 11, COLOR2FLAGS(RGB(255,255,255)));
      dc->drawSolidRect(BAR_LEFT + (i * 11), BAR_TOP_MARGIN + (j * 11), 11, 11, 1, COLOR2FLAGS(BLACK));
    }
  }
}

uint32_t PalletColorType::getRGB()
{
  return 0;
}

uint32_t PalletColorType::getBarValue(int bar, coord_t pos)
{
  return 0;
}

ColorEditor::ColorEditor(FormGroup *window, const rect_t rect, uint32_t color,
                         std::function<void (uint32_t rgb)> setValue) :
  FormGroup(window, rect),
  _setValue(std::move(setValue)),
  _color(color)
{

  // colorType = new HSVColorType(this, _color);
  colorType = new HSVColorType(this, color);

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
  new TextButton(this, a, "RGB", 
    [=] () {
      if (colorType != nullptr) {
        delete colorType;
      }
      colorType = new RGBColorType(this, color);
      invalidate();
      return 0;
    }, 0, COLOR_THEME_PRIMARY1);
  a.y = a.y + BUTTON_HEIGHT + BUTTON_MARGIN;
  lastButton = new TextButton(this, a, "PAL", 
    [=] () {
      if (colorType != nullptr) {
        delete colorType;
      }
      colorType = new PalletColorType(this, color);
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

