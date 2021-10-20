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
#include <stdlib.h>
#include <list>
#include <string>
#include <locale>
#include <algorithm>
#include "sdcard.h"
#include "colors.h"
#include "str_functions.h"

class ThemePersistance;
extern ThemePersistance themePersistance;

#define COLOR_COUNT 13

struct ColorEntry
{
    LcdColorIndex colorNumber;
    uint32_t colorValue;

    bool operator== (const ColorEntry &a) { return this->colorNumber == a.colorNumber; }
};


#define AUTHOR_LENGTH 50
#define INFO_LENGTH 255
#define NAME_LENGTH 50
class ThemeFile
{
 public:
    ThemeFile(std::string themePath) :
      path(themePath)
    {
        if (themePath.size()) {
            deSerialize();
        }
    }
    ThemeFile(const ThemeFile &theme)
    {
        path = theme.path;
        strncpy(name, theme.name, NAME_LENGTH);
        strncpy(author, theme.author, AUTHOR_LENGTH);
        strncpy(info, theme.info, INFO_LENGTH);
        colorList.assign(theme.colorList.begin(), theme.colorList.end());
    }

    void serialize();

    std::string getPath() { return path; }
    char *getName() { return name; }
    char *getAuthor() { return author; }
    char *getInfo() { return info; }

    ColorEntry *getColorEntryByIndex(LcdColorIndex colorNumber) {
        int n = 0;
        for (auto colorEntry : colorList) {
            if (colorEntry.colorNumber == colorNumber)
                return &colorList[n];
            n++;
        }

        return nullptr;
    }

    uint32_t getColorByName(std::string colorName) {

        auto colorIndex = findColorIndex(colorName.c_str());
        ColorEntry a = {colorIndex, 0};
        auto colorEntry = std::find(colorList.begin(), colorList.end(), a);
        if (colorEntry != colorList.end())
            return colorEntry->colorValue;
        
        return 0;
    }
    
    void setName(std::string name) { strcpy(this->name, name.c_str()); }
    void setAuthor(std::string author) { strcpy(this->author, author.c_str()); }
    void setInfo(std::string info) { strcpy(this->info, info.c_str()); }

    std::vector<ColorEntry> getColorList() { return colorList; }
    void setColor(LcdColorIndex colorIndex, uint32_t color);

    virtual std::string getThemeImageFileName();
    virtual void applyTheme();

  protected:
    FIL file;
    std::string path;
    char name[NAME_LENGTH];
    char author[AUTHOR_LENGTH];
    char info[INFO_LENGTH];
    std::vector<ColorEntry> colorList;


    enum ScanState
    {
        none,
        summary,
        colors
    };

    void deSerialize();
    bool convertRGB(char *pColorRGB, uint32_t &color);
    LcdColorIndex findColorIndex(const char *name);
    bool readNextLine(char * line, int maxlen);
};


class ThemePersistance
{
  public:
    ThemePersistance()
    {
    }

    static ThemePersistance *instance() {
        return &themePersistance;
    }

    void loadDefaultTheme();
    void setDefaultTheme(int index);
    void deleteDefaultTheme();
    char **getColorNames();


    std::vector<std::string> getNames()
    {
        std::vector<std::string> names;
        for (auto theme:themes) {
            names.emplace_back(theme->getName());
        }

        return names;
    }

    void applyTheme(int index)
    {
        auto theme = themes[index];
        theme->applyTheme();
    }

    inline int getThemeIndex() {return currentTheme;}
    inline void setThemeIndex(int index) { currentTheme = index;}

    inline ThemeFile* getCurrentTheme() { return themes[currentTheme]; }
    inline ThemeFile* getThemeByIndex(int index) { return themes[index]; }

    void refresh()
    {
        scanForThemes();
        insertDefaultTheme();
    }

  protected:
    std::vector<ThemeFile *> themes;
    int currentTheme = 0;
    void scanForThemes();
    void insertDefaultTheme();
};
