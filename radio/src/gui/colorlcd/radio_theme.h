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
#include "tabsgroup.h"
#include "color_editor.h"
#include "file_preview.h"
#include "file_carosell.h"

class ThemeColorPreview: public FormField
{
  public:
    ThemeColorPreview(Window *parent, const rect_t &rect, std::vector<ColorEntry> colorList) :
      FormField(parent, rect, NO_FOCUS),
      colorList(colorList)
    {
    }

    void setColorList(std::vector<ColorEntry> colorList)
    {
      this->colorList.assign(colorList.begin(), colorList.end());
      invalidate();
    }

    void paint(BitmapBuffer *dc) override
    {
      int totalNecessaryWidth = colorList.size() * (boxWidth + 2);
      int x = (rect.w - totalNecessaryWidth) / 2;
      for (auto color: colorList) {
        dc->drawSolidFilledRect(x, 0, boxWidth, boxWidth, COLOR2FLAGS(color.colorValue));
        dc->drawSolidRect(x, 0, boxWidth, boxWidth, 1, COLOR2FLAGS(BLACK));
        x += boxWidth + 2;
      }
    }

  protected:
    std::vector<ColorEntry> colorList;
    int boxWidth = 18;
};


class ThemeSetupPage: public PageTab {
  public:
    ThemeSetupPage();

    void build(FormWindow * window) override;

  protected:
    Window *previewWindow = nullptr;
    FileCarosell *fileCarosell = nullptr;
    ThemeColorPreview *themeColorPreview = nullptr;
    ListBox *listBox = nullptr;
    StaticText *authorText = nullptr;
    StaticText *authorLabel = nullptr;
    StaticText *nameLabel = nullptr;
    StaticText *nameText = nullptr;
    int currentTheme = 0;
};
