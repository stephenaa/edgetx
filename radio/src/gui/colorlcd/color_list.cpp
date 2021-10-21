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
#include "color_list.h"
#include "color_editor.h"

ColorList::ColorList(
    Window *parent, const rect_t &rect, std::vector<ColorEntry> colors,
    std::function<void(uint32_t value)> setValue,
    WindowFlags windowFlag, LcdFlags lcdFlags) :
    ListBase(parent, rect, getColorListNames(colors), nullptr, setValue),
    colorList(colors),
    tp(ThemePersistance::instance())
{
  setSelected(0);
  setLongPressHandler([=] (event_t event) {
    createColorEditorPopup();
  });
}

void ColorList::createColorEditorPopup()
{
  new ColorEditorPopup(this, 
    [=] () {
      return colorList[selected].colorValue;
    },
    [=] (uint32_t rgb) {
      colorList[selected].colorValue = rgb;
      _setValue(selected);
      invalidate();
    });
}

std::vector<std::string> ColorList::getColorListNames(std::vector<ColorEntry> colors)
{
  std::vector<std::string> names;
  char **colorNames = tp->getColorNames();
  for (auto color : colors) {
    names.emplace_back(colorNames[color.colorNumber]);
  }
  
  return names;
}

bool ColorList::onTouchEnd(coord_t x, coord_t y)
{
  ListBase::onTouchEnd(x, y);
  
  int selY = selected * lineHeight;
  if (x > rect.w - 22 && x < rect.w - 5 && y > selY && y < selY + lineHeight - 6) {
    createColorEditorPopup();
  }

  return true;
}

void ColorList::drawLine(BitmapBuffer *dc, const rect_t &rect, uint32_t index, LcdFlags lcdFlags)
{
  ListBase::drawLine(dc, rect, index, lcdFlags);
  dc->drawSolidFilledRect(rect.w - 22, rect.y, 16, lineHeight - 6,
                          COLOR2FLAGS(colorList[index].colorValue));
  dc->drawSolidRect(rect.w - 22, rect.y, 16, lineHeight - 6, 1, COLOR2FLAGS(BLACK));
}
