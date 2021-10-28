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
#include "color_editor2.h"

ColorList::ColorList(
    Window *parent, const rect_t &rect, std::vector<ColorEntry> colors,
    WindowFlags windowFlag, LcdFlags lcdFlags) :
    ListBase(parent, rect, getColorListNames(colors), nullptr, nullptr),
    colorList(colors),
    tp(ThemePersistance::instance())
{
  setSelected(0);
  setEditMode(true);
}

std::vector<std::string> ColorList::getColorListNames(std::vector<ColorEntry> colors)
{
  std::vector<std::string> names;
  char **colorNames = ThemePersistance::getColorNames();
  for (auto color : colors) {
    names.emplace_back(colorNames[color.colorNumber]);
  }
  
  return names;
}

void ColorList::drawLine(BitmapBuffer *dc, const rect_t &rect, uint32_t index, LcdFlags lcdFlags)
{
  ListBase::drawLine(dc, rect, index, lcdFlags);
  dc->drawSolidFilledRect(rect.w - 22, rect.y, 16, lineHeight - 6,
                          COLOR2FLAGS(colorList[index].colorValue));
  dc->drawSolidRect(rect.w - 22, rect.y, 16, lineHeight - 6, 1, COLOR2FLAGS(BLACK));
}
