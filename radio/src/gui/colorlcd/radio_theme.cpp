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
#include "radio_theme.h"
#include <algorithm>

#include "bitmapbuffer.h"
#include "libopenui.h"
#include "opentx.h"
#include "file_preview.h"
#include "theme_manager.h"
#include "themes/480_bitmaps.h"
#include "view_main.h"
#include "widget_settings.h"
#include "sliders.h"
#include "color_list.h"
#include "color_editor.h"
#include "color_editor2.h"
#include "listbox.h"

#if (LCD_W > LCD_H)
  #define COLOR_PREVIEW_WIDTH 18
  #define LEFT_LIST_WIDTH (LCD_W / 2) - COLOR_PREVIEW_WIDTH
  #define LEFT_LIST_HEIGHT (LCD_H - TOPBAR_HEIGHT - 38)
#else
  #define LEFT_LIST_WIDTH LCD_W
  #define LEFT_LIST_HEIGHT (LCD_H / 2 - 38)
#endif

#define MARGIN_WIDTH 5


// class to hold a color list and apply / restore it while drawing standard controls
class ColorMaintainer
{
  public:
    void setColorList(std::vector<ColorEntry> colorList)
    {
      this->colorList.assign(colorList.begin(), colorList.end());
    }


    void applyColorValues()
    {
      // save old values;
      for (auto i = 0; i < COLOR_COUNT; i++) {
        oldColorVals[i] = lcdColorTable[i];
      }

      for (auto color: colorList) {
        lcdColorTable[color.colorNumber] = color.colorValue;
      }
    }

    void restoreColorValues()
    {
      for (auto i = 0; i < COLOR_COUNT; i++) {
        lcdColorTable[i] = oldColorVals[i];
      }
    }

  protected:
    uint32_t oldColorVals[COLOR_COUNT];
    std::vector<ColorEntry> colorList;
};

ColorMaintainer colorMaintainer;

// override of text edit to draw it using a selected theme
class ThemedTextEdit : public TextEdit
{
  public:
    using TextEdit::TextEdit;

  void paint(BitmapBuffer *dc) override
  {
      colorMaintainer.applyColorValues();
      TextEdit::paint(dc);
      colorMaintainer.restoreColorValues();
  }
};

class ThemedStaticText : public StaticText
{
  public:
    using StaticText::StaticText;

  void paint(BitmapBuffer *dc) override
  {
      colorMaintainer.applyColorValues();
      setTextFlags(COLOR_THEME_PRIMARY1);
      StaticText::paint(dc);
      colorMaintainer.restoreColorValues();
  }
};

class ThemedCheckBox : public CheckBox
{
  public:
    ThemedCheckBox(Window *parent, rect_t rect, bool checked) :
      CheckBox(parent, rect, [=] () { return checked; }, [] (uint8_t value) {}),
      checked(checked)
      {
      }
    
    void paint(BitmapBuffer * dc) override
    {
      colorMaintainer.applyColorValues();
      CheckBox::paint(dc);
      colorMaintainer.restoreColorValues();
    }
  protected:
    bool checked;
};

class ThemedMainViewHorizontalTrim : public MainViewHorizontalTrim
{
  public:
    using MainViewHorizontalTrim::MainViewHorizontalTrim;
    void paint (BitmapBuffer *dc) override 
    {
      colorMaintainer.applyColorValues();
      MainViewHorizontalTrim::paint(dc);
      colorMaintainer.restoreColorValues();
    }
};

class ThemedMainViewHorizontalSlider: public MainViewHorizontalSlider
{
  public: 
    using MainViewHorizontalSlider::MainViewHorizontalSlider;
    void paint (BitmapBuffer *dc) override
    {
      colorMaintainer.applyColorValues();
      MainViewHorizontalSlider::paint(dc);
      colorMaintainer.restoreColorValues();
    }
};

class PreviewWindow : public FormGroup
{
  friend class ThemedCheckBox;

  public:
    PreviewWindow(Window *window, rect_t rect, std::vector<ColorEntry> colorList) :
        FormGroup(window, rect, NO_FOCUS), 
        colorList(colorList)
    {
      new ThemedStaticText(this, {5, 40, 100, LINE_HEIGHT}, "Checkbox", 0, COLOR_THEME_PRIMARY1);
      new ThemedCheckBox(this, {100 + 15, 40, 20, LINE_HEIGHT}, true);
      new ThemedCheckBox(this, {140 + 15, 40, 20, LINE_HEIGHT}, false);
      new ThemedMainViewHorizontalTrim(this, {5, 65, HORIZONTAL_SLIDERS_WIDTH, 20}, 0);
      new ThemedMainViewHorizontalSlider(this, {5, 87, HORIZONTAL_SLIDERS_WIDTH, 20}, 0);
      colorMaintainer.setColorList(colorList);
    }

    BitmapBuffer *getBitmap(const uint8_t *maskData, uint32_t bgColor,
                            uint32_t fgColor, int *width)
    {
      auto mask = BitmapBuffer::load8bitMask(maskData);
      BitmapBuffer *newBm = new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());
      newBm->clear(bgColor);
      newBm->drawMask(0, 0, mask, fgColor);
      return newBm;
    }


    void drawTime(BitmapBuffer *dc)
    {
      // time on top bar
      struct gtm t;
      char str[10];
      char str1[20];
      const char * const STR_MONTHS[] = TR_MONTHS;

      gettime(&t);
      sprintf(str, "%d %s\n", t.tm_mday, STR_MONTHS[t.tm_mon]);

      getTimerString(str1, getValue(MIXSRC_TX_TIME));
      strcat(str, str1);
      dc->drawText(rect.w - 40, 5, str, COLOR_THEME_PRIMARY2 | FONT(XS));
    }

    void paint(BitmapBuffer *dc) override
    {
      colorMaintainer.applyColorValues();

      // background
      dc->clear(COLOR_THEME_SECONDARY3);

      // top bar background
      dc->drawSolidFilledRect(0, 0, rect.w, TOPBAR_HEIGHT, COLOR_THEME_SECONDARY1);

      int width;
      int x = 5;
      // topbar icons
      auto bm = getBitmap(mask_menu_radio, COLOR_THEME_SECONDARY1, COLOR_THEME_PRIMARY2, &width);
      dc->drawBitmap(x, 5, bm);
      x += MENU_HEADER_BUTTON_WIDTH + 2;
      delete bm;

      bm = getBitmap(mask_radio_tools, COLOR_THEME_SECONDARY1, COLOR_THEME_PRIMARY2, &width);
      dc->drawBitmap(x, 5, bm);
      x += MENU_HEADER_BUTTON_WIDTH + 2;
      delete bm;

      bm = getBitmap(mask_radio_setup, COLOR_THEME_SECONDARY1, COLOR_THEME_PRIMARY2, &width);
      dc->drawBitmap(x, 5, bm);
      delete bm;

      drawTime(dc);

      colorMaintainer.restoreColorValues();
    }

  protected:
    std::vector<ColorEntry> colorList;
    CheckBox *checkBox;
};

#if (LCD_W > LCD_H)
  constexpr int BUTTON_HEIGHT = 30;
  constexpr int BUTTON_WIDTH  = 75;
  constexpr LcdFlags textFont = FONT(STD);
  constexpr rect_t detailsDialogRect = {50, 50, 400, 170};
  constexpr int labelWidth = 150;
#else
  constexpr int BUTTON_HEIGHT = 25;
  constexpr int BUTTON_WIDTH  = 50;
  constexpr LcdFlags textFont = FONT(XS);
  constexpr rect_t detailsDialogRect = {5, 50, LCD_W - 10, 340};
  constexpr int labelWidth = 120;
#endif


#define COLOR_LIST_WIDTH ((LCD_W * 3)/10)
#define COLOR_LIST_HEIGHT (LCD_H - TOPBAR_HEIGHT)

class ThemeDetailsDialog: public Dialog
{
  public:
    ThemeDetailsDialog(Window *parent, ThemeFile theme, std::function<void (ThemeFile theme)> saveHandler = nullptr) :
      Dialog(parent, STR_EDIT_THEME_DETAILS, detailsDialogRect),
      theme(theme),
      saveHandler(saveHandler)
    {
      FormGridLayout grid(detailsDialogRect.w);
      grid.setLabelWidth(labelWidth);
      grid.spacer(8);

      new StaticText(&content->form, grid.getLabelSlot(), STR_NAME, 0, COLOR_THEME_PRIMARY1);
      new TextEdit(&content->form, grid.getFieldSlot(), this->theme.getName(), NAME_LENGTH);
      grid.nextLine();
      new StaticText(&content->form, grid.getLabelSlot(), STR_AUTHOR, 0, COLOR_THEME_PRIMARY1);
      new TextEdit(&content->form, grid.getFieldSlot(), this->theme.getAuthor(), AUTHOR_LENGTH);
      grid.nextLine();
      new StaticText(&content->form, grid.getLineSlot(), STR_DESCRIPTION, 0, COLOR_THEME_PRIMARY1);
      grid.nextLine();
      new TextEdit(&content->form, grid.getLineSlot(), this->theme.getInfo(), INFO_LENGTH);
      grid.nextLine();

      rect_t r = {detailsDialogRect.w - (BUTTON_WIDTH + 5), grid.getWindowHeight() + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
      new TextButton(&content->form, r, STR_SAVE, [=] () {
        if (saveHandler != nullptr)
          saveHandler(this->theme);
        deleteLater();
        return 0;
      }, BUTTON_BACKGROUND | OPAQUE, textFont);
      r.x -= (BUTTON_WIDTH + 5);
      new TextButton(&content->form, r, STR_CANCEL, [=] () {
        deleteLater();
        return 0;
      }, BUTTON_BACKGROUND | OPAQUE, textFont);
    }
  protected:
    ThemeFile theme;
    std::function<void(ThemeFile theme)> saveHandler = nullptr;
};

class ColorEditPage : public Page
{
public:
  ColorEditPage(ThemeFile theme, LcdColorIndex indexOfColor, 
                std::function<void (ThemeFile *theme)> saveHandler = nullptr) :
    Page(ICON_RADIO_EDIT_THEME),
    _saveHandler(std::move(saveHandler)),
    _indexOfColor(indexOfColor),
    _theme(theme)
  {
    _originalColor = *theme.getColorEntryByIndex(_indexOfColor);
    buildBody(&body); // this must be called first
    buildHead(&header);
  }

  void deleteLater(bool detach = true, bool trash = true) override
  {
    // restore the orignal color because we did not save
    if (!bSaved) {
      _theme.setColor(_indexOfColor, _originalColor.colorValue);
      colorMaintainer.setColorList(_theme.getColorList());
    }
    Page::deleteLater();
  }


protected:
  bool bSaved = false;
  ColorEntry _originalColor;
  std::function<void (ThemeFile *theme)> _saveHandler;
  LcdColorIndex _indexOfColor;
  ThemeFile  _theme;
  TextButton *saveButton;
  TextButton *cancelButton;
  ColorEditor *colorEditor;
  PreviewWindow *previewWindow;

  void buildBody(FormWindow* window)
  {
    rect_t r;
    r = { 0, 4, COLOR_LIST_WIDTH, COLOR_LIST_HEIGHT };
    colorEditor = new ColorEditor(window, r,  _theme.getColorEntryByIndex(_indexOfColor)->colorValue,
      [=](uint32_t rgb) {
        _theme.setColor(_indexOfColor, rgb);
        if (previewWindow != nullptr) {
          colorMaintainer.setColorList(_theme.getColorList());
          previewWindow->invalidate();
        }
      });

    if (LCD_W > LCD_H) {
      r = { COLOR_LIST_WIDTH + MARGIN_WIDTH, 4, LCD_W - COLOR_LIST_WIDTH - MARGIN_WIDTH * 2, COLOR_LIST_HEIGHT };
    }
    else {
      r = { 0, LEFT_LIST_HEIGHT + 4,  LEFT_LIST_WIDTH, LEFT_LIST_HEIGHT - 4 };
    }
    previewWindow = new PreviewWindow(window, r, _theme.getColorList());
  }

  void buildHead(PageHeader* window)
  {
    LcdFlags flags = 0;
    if (LCD_W < LCD_H) {
      flags = FONT(XS);
    }

    // page title
    new StaticText(window,
      { PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
       PAGE_LINE_HEIGHT },
      "Edit Color", 0, COLOR_THEME_PRIMARY2 | flags);
    new StaticText(window,
      { PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
       LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT },
      ThemePersistance::getColorNames()[(int)_indexOfColor], 0, COLOR_THEME_PRIMARY2 | flags);

    // save and cancel
    rect_t r = { LCD_W - (BUTTON_WIDTH + 5), 6, BUTTON_WIDTH, BUTTON_HEIGHT };
    saveButton = new TextButton(window, r, STR_SAVE, [=]() {
      bSaved = true;
      if (_saveHandler != nullptr) {
        _saveHandler(&_theme);
      }
      deleteLater();
      return 0;
      }, BUTTON_BACKGROUND | OPAQUE, textFont);
    r.x -= (BUTTON_WIDTH + 5);

    // setup the prev next controls so save and details are in the mix
    window->link(saveButton, colorEditor);
  }
};

class ThemeEditPage : public Page
{
  public:
    explicit ThemeEditPage(ThemeFile theme, std::function<void (ThemeFile &theme)> saveHandler = nullptr) :
      Page(ICON_RADIO_EDIT_THEME),
      _theme(theme),
      page(this),
      saveHandler(std::move(saveHandler))
    {
      buildBody(&body);
      buildHeader(&header);
    }

    void buildHeader(FormGroup *window)
    {
      LcdFlags flags = 0;
      if (LCD_W < LCD_H) {
        flags = FONT(XS);
      }

      // page title
      new StaticText(window,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
                     STR_EDIT_THEME, 0, COLOR_THEME_PRIMARY2 | flags);
      themeName = new StaticText(window,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                      LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                     _theme.getName(), 0, COLOR_THEME_PRIMARY2 | flags);

      // save and cancel
      rect_t r = {LCD_W - (BUTTON_WIDTH + 5), 6, BUTTON_WIDTH, BUTTON_HEIGHT };
      saveButton = new TextButton(window, r, STR_SAVE, [=] () {
        if (saveHandler != nullptr)
          saveHandler(_theme);
        deleteLater();
        return 0;
      }, BUTTON_BACKGROUND | OPAQUE, textFont);
      r.x -= (BUTTON_WIDTH + 5);
      detailButton = new TextButton(window, r, STR_DETAILS, [=] () {
        new ThemeDetailsDialog(page, _theme, [=] (ThemeFile t) {
          _theme.setAuthor(t.getAuthor());
          _theme.setInfo(t.getInfo());
          _theme.setName(t.getName());

          // update the theme name
          themeName->setText(_theme.getName());
        });
        return 0;
      }, BUTTON_BACKGROUND | OPAQUE, textFont);

      // setup the prev next controls so save and details are in the mix
      window->link(cList, detailButton);
      window->link(saveButton, cList);
    }

    void buildBody(FormGroup *window)
    {
      rect_t r = { 0, 4, COLOR_LIST_WIDTH, COLOR_LIST_HEIGHT};
      cList = new ColorList(window, r, _theme.getColorList());
      cList->setLongPressHandler([=] (event_t event) {
        auto colorEntry = cList->getSelectedColor();
        new ColorEditPage(_theme, colorEntry.colorNumber, 
        [=] (ThemeFile *theme) {
          if (previewWindow != nullptr && cList != nullptr) {
            _theme.setColor(colorEntry.colorNumber, 
                            theme->getColorEntryByIndex(colorEntry.colorNumber)->colorValue);
            cList->setColorList(_theme.getColorList());
            colorMaintainer.setColorList(_theme.getColorList());
            previewWindow->invalidate();
          }
        });
      });

      if (LCD_W > LCD_H) {
        r = { COLOR_LIST_WIDTH + MARGIN_WIDTH, 4, LCD_W - COLOR_LIST_WIDTH - MARGIN_WIDTH*2, COLOR_LIST_HEIGHT };
      } else {
        r = { 0, LEFT_LIST_HEIGHT + 4,  LEFT_LIST_WIDTH, LEFT_LIST_HEIGHT - 4};
      }
      previewWindow = new PreviewWindow(window, r, _theme.getColorList());
    }

  protected:
    ThemeFile _theme;
    Page *page;
    std::function<void(ThemeFile &theme)> saveHandler = nullptr;
    PreviewWindow *previewWindow = nullptr;
    ColorList *cList = nullptr;
    StaticText *themeName = nullptr;
    TextButton *saveButton;
    TextButton *detailButton;
};

ThemeSetupPage::ThemeSetupPage() : PageTab(STR_THEME_EDITOR, ICON_RADIO_EDIT_THEME) {}

void ThemeSetupPage::build(FormWindow *window)
{
  auto tp = ThemePersistance::instance();
  auto theme = tp->getCurrentTheme();
  currentTheme = tp->getThemeIndex();

  themeColorPreview = nullptr;
  listBox = nullptr;
  fileCarosell = nullptr;
  nameText = nullptr;
  authorText = nullptr;
  
  rect_t r = { 0, 4, LEFT_LIST_WIDTH, LEFT_LIST_HEIGHT };

  listBox = new ListBox(
    window, r, tp->getNames(),
      [=]() { 
        return currentTheme;
      },
      [=](uint8_t value) {
        if (themeColorPreview && authorText && nameText && fileCarosell) {
          ThemeFile *theme = tp->getThemeByIndex(value);
          themeColorPreview->setColorList(theme->getColorList());
          authorText->setText(theme->getAuthor());
          nameText->setText(theme->getName());
          fileCarosell->setFileNames(theme->getThemeImageFileNames());
        }
        currentTheme = value;
      });
  listBox->setTitle(STR_THEME + std::string("s"));
  listBox->setLongPressHandler([=] (event_t event) {
    auto menu = new Menu(window,false);

    // you cant edit the default theme
    menu->addLine(STR_ACTIVATE, [=]() {
      tp->applyTheme(listBox->getSelected());
      tp->setDefaultTheme(listBox->getSelected());
      nameText->setTextFlags(COLOR_THEME_PRIMARY1);
      authorText->setTextFlags(COLOR_THEME_PRIMARY1);
      nameLabel->setTextFlags(COLOR_THEME_PRIMARY1);
      authorLabel->setTextFlags(COLOR_THEME_PRIMARY1);
    });
    if (listBox->getSelected() != 0) {
      menu->addLine(STR_EDIT,
        [=] () {
          auto theme = tp->getThemeByIndex(currentTheme);
          if (theme == nullptr) return;
          
          new ThemeEditPage(*theme, 
            [=](ThemeFile &theme) {
              auto t = tp->getThemeByIndex(currentTheme);
              if (t != nullptr) {
                t->setName(theme.getName());
                t->setAuthor(theme.getAuthor());
                t->setInfo(theme.getInfo());

                // update the colors that were edited
                int n = 0;
                for (auto color : theme.getColorList()) {
                  t->setColorByIndex(n, color.colorValue);
                  n++;
                }

                // the list of theme names might have changed
                listBox->setNames(tp->getNames());
              }
            });
      });
    }
    menu->addLine(STR_NEW, [=] () {});
    if (listBox->getSelected() != 0) {
      menu->addLine(STR_DELETE, [=] () {});
    }
  });
  listBox->setEditMode(true);

  if (LCD_W > LCD_H) {
    r.x = LEFT_LIST_WIDTH + MARGIN_WIDTH;
    r.w = LCD_W - r.x;
  } else {
    r.y += LEFT_LIST_HEIGHT + 4;
    r.x = 0;
    r.w = LCD_W;
  }

  auto colorList = theme != nullptr ? theme->getColorList() : std::vector<ColorEntry>();
  rect_t colorPreviewRect = {LEFT_LIST_WIDTH + 2, 0, COLOR_PREVIEW_WIDTH, LEFT_LIST_HEIGHT};
  themeColorPreview = new ThemeColorPreview(window, colorPreviewRect, colorList);
  
  r.h = 130;
  r.w -= COLOR_PREVIEW_WIDTH - 4;
  r.x += COLOR_PREVIEW_WIDTH;
  auto fileNames = theme != nullptr ? theme->getThemeImageFileNames() : std::vector<std::string>();
  fileCarosell = new FileCarosell(window, r, fileNames, listBox);

  r.y += 130;
  r.h = 15;
  nameLabel = new StaticText(window, r, STR_NAME, 0, COLOR_THEME_PRIMARY1 | FONT(XS)| FONT(BOLD));
  r.y += 16;
  nameText = new StaticText(window, r, theme != nullptr ? theme->getName() : "", 0, 
                            COLOR_THEME_PRIMARY1 | FONT(XS));
  r.y += 16;
  authorLabel = new StaticText(window, r, STR_AUTHOR, 0, COLOR_THEME_PRIMARY1 | FONT(XS) | FONT(BOLD));
  r.y += 16;
  authorText = new StaticText(window, r, theme != nullptr ? theme->getAuthor() : "", 0, 
                              COLOR_THEME_PRIMARY1 | FONT(XS));
}
