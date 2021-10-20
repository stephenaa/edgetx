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
#include "file_carosell.h"

FileCarosell::FileCarosell(Window *parent, const rect_t &rect,
                           std::vector<std::string> fileNames, FormField *nextCtrl) :
    FormGroup(parent, rect, NO_FOCUS | FORM_NO_BORDER),
    _fileNames(fileNames),
    fp(new FilePreview(parent, {rect.x + 20, rect.y, rect.w - 40, rect.h - 40},
                       false))
{
  auto tbPrev = new TextButton(
      this, {5, rect.h / 2 - 20, 20, 30}, "<",
      [=]() {
        int newSelected = selected - 1;
        if (newSelected < 0) newSelected = _fileNames.size() - 1;
        setSelected(newSelected);
        return 0;
      },
      BUTTON_BACKGROUND, COLOR_THEME_PRIMARY1);
  auto tbNext = new TextButton(
      this, {rect.w - 30, rect.h / 2 - 20, 20, 30}, ">",
      [=]() {
        int newSelected = (selected + 1) % _fileNames.size();
        setSelected(newSelected);
        return 0;
      },
      BUTTON_BACKGROUND, COLOR_THEME_PRIMARY1);

  setSelected(0);

  tbPrev->link(tbNext, nextCtrl);
  tbNext->link(nextCtrl, tbPrev);
  setFocusHandler([=] (bool focus) {
    if (focus) {
      (*getChildren().begin())->setFocus();
    }
  });
}
