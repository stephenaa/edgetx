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
#include "color_editor.h"
#include "listbox.h"


// class to hold a color list and apply / restore it while drawing standard controls
class ColorMaintainer
{
  public:
    void setColorList(std::vector<ColorEntry> colorList)
    {
      this->colorList.assign(colorList.begin(), colorList.end());
    }


    void applyColorList()
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
      colorMaintainer.applyColorList();
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
      colorMaintainer.applyColorList();
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
      colorMaintainer.applyColorList();
      CheckBox::paint(dc);
      colorMaintainer.restoreColorValues();
    }
  protected:
    bool checked;
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
      colorMaintainer.applyColorList();

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

ThemeSetupPage::ThemeSetupPage() : PageTab("Theme Editor", ICON_MODEL_NOTES) {}

void ThemeSetupPage::build(FormWindow *window)
{
  FormGridLayout grid(LCD_W, 2);

  auto tp = ThemePersistance::instance();
  auto theme = tp->getCurrentTheme();

  currentTheme = tp->getThemeIndex();
  rect_t r = grid.getLabelSlot();
  r.h += LINE_HEIGHT * 2;
  auto lb = new ListBox(
      window, r, tp->getNames(), [=]() { return currentTheme; },
      [=](uint8_t value) {
        if (colorEditor != nullptr && value != currentTheme) {
          ColorEditor *ce = static_cast<ColorEditor *>(this->colorEditor);
          ce->setTheme(tp->getThemeByIndex(value));
          colorMaintainer.setColorList(tp->getThemeByIndex(value)->getColorList());

          previewWindow->invalidate();
          currentTheme = value;
        }
      });
  lb->setTitle("Themes");
  lb->setLongPressHandler([=] (event_t event) {
      auto menu = new Menu(window,false);
      menu->setCloseWhenClickOutside(false);
      menu->addLine("Save", [=] () {
        TRACE("SELECTED");
      });
      menu->addLine("New", [=] () {
        TRACE("SELECTED");
      });
      menu->addLine("Details...", [=] () {
        TRACE("SELECTED");
      });
      menu->addLine("Delete", [=] () {
        TRACE("SELECTED");
      });
  });

  grid.spacer(LINE_HEIGHT * 3 + 5);
  r = grid.getLabelSlot();
  r.h += LINE_HEIGHT * 6 - 5;
  colorEditor = new ColorEditor(window, r, theme, 
    [=] () {
      ColorEditor *ce = static_cast<ColorEditor *>(this->colorEditor);
      colorMaintainer.setColorList(ce->getColorList());
      previewWindow->invalidate();
    },
    0, COLOR_THEME_PRIMARY1);

  r = grid.getFieldSlot();
  r.x += 5; r.w -= 5; r.y = 0; r.h = window->getRect().h;
  previewWindow = new PreviewWindow(window, r, theme->getColorList());

  grid.spacer(45);
  window->setInnerHeight(grid.getWindowHeight());
}
