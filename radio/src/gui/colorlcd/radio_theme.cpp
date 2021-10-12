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

#define LINE_HEIGHT (coord_t) PAGE_LINE_HEIGHT
#define SET_DIRTY() setDirty()

BitmapBuffer *getBitmap(const uint8_t *maskData, uint32_t bgColor,
                        uint32_t fgColor, int *width)
{
  auto mask = BitmapBuffer::load8bitMask(maskData);
  BitmapBuffer *newBm =
      new BitmapBuffer(BMP_RGB565, mask->width(), mask->height());
  newBm->clear(bgColor);
  newBm->drawMask(0, 0, mask, fgColor);
  return newBm;
}

class PreviewWindow : public FormGroup
{
 public:
  PreviewWindow(Window *window, rect_t rect,
                std::vector<ColorEntry> colorList) :
      FormGroup(window, rect), colorList(colorList)
  {
  }

  void setColorList(std::vector<ColorEntry> colorList)
  {
    this->colorList = colorList;
    invalidate();
  }

uint32_t oldColorVals[COLOR_COUNT];
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



  void paint(BitmapBuffer *dc) override
  {
    applyColorList();
    // ColorEntry colorBackground = {COLOR_THEME_SECONDARY3_INDEX, 0};
    // ColorEntry topBar = {COLOR_THEME_SECONDARY1_INDEX, 0};
    // ColorEntry primary2 = {COLOR_THEME_PRIMARY2_INDEX, 0};
    // auto colorBackgroundEntry =
    //     std::find(colorList.begin(), colorList.end(), colorBackground);
    // auto colorTopBarEntry =
    //     std::find(colorList.begin(), colorList.end(), topBar);
    // auto primary2Entry =
    //     std::find(colorList.begin(), colorList.end(), primary2);

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

    // time on top bar
    struct gtm t;
    gettime(&t);
    char str[10];
    char str1[20];
#if defined(TRANSLATIONS_CN) || defined(TRANSLATIONS_TW)
      sprintf(str, "%02d-%02d", t.tm_mon + 1, t.tm_mday);
#else
      const char * const STR_MONTHS[] = TR_MONTHS;
      sprintf(str, "%d %s\n", t.tm_mday, STR_MONTHS[t.tm_mon]);
#endif

    getTimerString(str1, getValue(MIXSRC_TX_TIME));
    strcat(str, str1);
    dc->drawText(rect.w - 40, 5, str, COLOR_THEME_PRIMARY2 | FONT(XS));

    restoreColorValues();
  }

 protected:
  std::vector<ColorEntry> colorList;
};

class ColorEntryEditor : public FormGroup
{
#define PREVIEW_Y_OFFSET 5
#define PREVIEW_HEIGHT 22
#define FIRST_EDIT (PREVIEW_HEIGHT + PREVIEW_Y_OFFSET + 3)
#define SECOND_EDIT (FIRST_EDIT + LINE_HEIGHT)
#define THIRD_EDIT (SECOND_EDIT + LINE_HEIGHT)
 public:
  ColorEntryEditor(Window *window, rect_t rect, std::function<void(ColorEntry value)> setValue) : 
    FormGroup(window, rect),
    setValue(setValue)
  {
    new StaticText(this, {5, FIRST_EDIT, 10, LINE_HEIGHT}, "R", 0,
                   COLOR_THEME_PRIMARY1);
    rEdit = new NumberEdit(this, {25, FIRST_EDIT, 40, LINE_HEIGHT}, 0, 255,
                           GET_SET_DEFAULT(r), 0, COLOR_THEME_PRIMARY1);
    new StaticText(this, {5, SECOND_EDIT, 10, LINE_HEIGHT}, "G", 0,
                   COLOR_THEME_PRIMARY1);
    gEdit = new NumberEdit(this, {25, SECOND_EDIT, 40, LINE_HEIGHT}, 0, 255,
                           GET_SET_DEFAULT(g), 0, COLOR_THEME_PRIMARY1);
    new StaticText(this, {5, THIRD_EDIT, 10, LINE_HEIGHT}, "B", 0,
                   COLOR_THEME_PRIMARY1);
    bEdit = new NumberEdit(this, {25, THIRD_EDIT, 40, LINE_HEIGHT}, 0, 255,
                           GET_SET_DEFAULT(b), 0, COLOR_THEME_PRIMARY1);
  }

  void setDirty() { 
    invalidate();
    colorEntry.colorValue = RGB(r, g, b);
    setValue(colorEntry);
  }

  void paint(BitmapBuffer *dc)
  {
    FormGroup::paint(dc);

    if (colorEntry.colorNumber != COLOR_COUNT)
      dc->drawSolidFilledRect(5, PREVIEW_Y_OFFSET, rect.w - 10, PREVIEW_HEIGHT,
                              COLOR2FLAGS(RGB(r, g, b)));

    dc->drawSolidRect(5, PREVIEW_Y_OFFSET, rect.w - 10, PREVIEW_HEIGHT, 1,
                      COLOR2FLAGS(BLACK));
  }

  void setColorToEdit(ColorEntry colorEntry)
  {
    this->colorEntry = colorEntry;
    r = GET_RED(colorEntry.colorValue);
    g = GET_GREEN(colorEntry.colorValue);
    b = GET_BLUE(colorEntry.colorValue);
    invalidate();
  }

 protected:
  ColorEntry colorEntry = {LCD_COLOR_COUNT, 0};
  NumberEdit *rEdit;
  NumberEdit *gEdit;
  NumberEdit *bEdit;
  uint32_t r = 0;
  uint32_t g = 0;
  uint32_t b = 0;
  std::function<void(ColorEntry value)> setValue;
};

class ColorList : public FormField
{
 public:
  ColorList(
      Window *parent, const rect_t &rect, std::vector<ColorEntry> colors,
      std::function<void(ColorEntry value)> setValue,
      WindowFlags windowFlags = 0, LcdFlags lcdFlags = 0) :
      FormField(parent, rect, windowFlags, lcdFlags),
      _setValue(std::move(setValue)),
      colorList(colors)
  {
    tp = ThemePersistance::instance();
    setInnerHeight(colors.size() * LINE_HEIGHT);
    setSelected(0);
  }

  ColorEntry getSelectedColor()
  {
    return colorList[selected];
  }

  void setSelected(int selected)
  {
    this->selected = selected;
    setScrollPositionY(LINE_HEIGHT * this->selected - LINE_HEIGHT);
    invalidate();
    _setValue(colorList[selected]);
  }

  void setColorList(std::vector<ColorEntry> colorList)
  {
    this->colorList = colorList;
    invalidate();
  }

  void paint(BitmapBuffer *dc) override
  {
    auto colorNames = tp->getColorNames();
    uint32_t bgColor = 0;
    if (hasFocus()) {
      bgColor = COLOR_THEME_FOCUS;
    } else {
      bgColor = COLOR_THEME_PRIMARY2;
    }

    int curY = 0;
    int n = 0;
    for (auto color : colorList) {
      if (n == selected) {
        dc->drawSolidFilledRect(1, curY, rect.w - 2, LINE_HEIGHT, bgColor);
      }

      LcdFlags textColor =
          n == selected ? COLOR_THEME_SECONDARY2 : COLOR_THEME_PRIMARY1;

      char *colorName = colorNames[color.colorNumber];
      dc->drawText(5, curY + 2, colorName, textColor);

      dc->drawSolidFilledRect(rect.w - 22, curY + 2, 16, LINE_HEIGHT - 6,
                              COLOR2FLAGS(color.colorValue));
      dc->drawSolidRect(rect.w - 22, curY + 2, 16, LINE_HEIGHT - 6, 1, COLOR2FLAGS(BLACK));
      curY += LINE_HEIGHT;
      n++;
    }

    if (!(windowFlags & (FORM_NO_BORDER | FORM_FORWARD_FOCUS))) {
      if (!editMode && hasFocus()) {
        dc->drawSolidRect(0, getScrollPositionY(), rect.w, rect.h, 2,
                          COLOR_THEME_FOCUS);
      } else if (!(windowFlags & FORM_BORDER_FOCUS_ONLY)) {
        dc->drawSolidRect(0, getScrollPositionY(), rect.w, rect.h, 1,
                          COLOR_THEME_SECONDARY2);
      }
    }
  }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override
  {
    if (!isEditMode()) return FormField::onEvent(event);

    int oldSelected = selected;
    switch (event) {
      case EVT_KEY_FIRST(KEY_EXIT):
        setEditMode(false);
        changeEnd();
        onKeyPress();
        setFocus(SET_FOCUS_DEFAULT, this);
        break;

      case EVT_KEY_BREAK(KEY_ENTER):
        if (isEditMode()) changeEnd();
        setEditMode(!isEditMode());
        onKeyPress();
        break;
      case EVT_ROTARY_RIGHT:
        oldSelected = (selected + 1) % colorList.size();
        setSelected(oldSelected);
        onKeyPress();
        break;
      case EVT_ROTARY_LEFT:
        oldSelected--;
        if (oldSelected < 0) {
          oldSelected = colorList.size() - 1;
        }
        setSelected(oldSelected);
        onKeyPress();
        break;
      default:
        FormField::onEvent(event);
    }
  }
#endif

#if defined(HARDWARE_TOUCH)
  bool onTouchEnd(coord_t x, coord_t y) override
  {
    if (!isEnabled()) return false;

    if (!hasFocus()) {
      setFocus(SET_FOCUS_DEFAULT);
    }

    if (!isEditMode()) setEditMode(true);

    selected = y / LINE_HEIGHT;
    setSelected(selected);
    return true;
    ;
  }
#endif

 protected:
  std::function<void(ColorEntry value)> _setValue;
  std::vector<ColorEntry> colorList;
  int selected = 0;
  ThemePersistance *tp;
};

class ColorEditor : public FormGroup
{
 public:
  ColorEditor(Window *window, rect_t rect, ThemeFile *theme,
              std::function<void()> update = nullptr,
              WindowFlags windowFlags = 0, LcdFlags lcdFlags = 0) :
      FormGroup(window, rect, windowFlags), 
      themeColorList(theme->getColorList()),
      theme(theme),
      update(update)
  {
    rect_t r = {rect.x + 2, rect.y + LINE_HEIGHT + 2, (rect.w * 3) / 5 - 2,
                rect.h - LINE_HEIGHT - 4};
    colorListWindow = new ColorList(
        window, r, theme->getColorList(),
        [=](ColorEntry value) {
          if (colorEntryEditor != nullptr) {
            auto found = std::find(themeColorList.begin(), themeColorList.end(), value);
            // change which color is editing
            if (found != themeColorList.end()) {
              colorEntryEditor->setColorToEdit(*found);
            }
          }
        },
        windowFlags, lcdFlags);

    r = {rect.x + 10 + (rect.w * 3) / 5, rect.y + LINE_HEIGHT,
         ((rect.w * 2) / 5) - 12, rect.h - LINE_HEIGHT - 2};
    colorEntryEditor = new ColorEntryEditor(window, r, [=] (ColorEntry value) {
      auto found = std::find(this->themeColorList.begin(), this->themeColorList.end(), value);
      if (found != this->themeColorList.end()) {
        found->colorValue = value.colorValue;
        this->colorListWindow->setColorList(themeColorList);
        if (update != nullptr)
          update();
      }
    });
  }

  void paint(BitmapBuffer *dc) override
  {
    FormGroup::paint(dc);

    dc->drawSolidFilledRect(2, 2, rect.w - 4, LINE_HEIGHT, COLOR_THEME_SECONDARY2);
    dc->drawText(rect.w / 2, 4, "Theme Colors", COLOR_THEME_PRIMARY2 | CENTERED);
  }

  std::vector<ColorEntry> getColorList()
  {
    return themeColorList;
  }

  void setTheme(ThemeFile *theme)
  {
    // make a copy of the theme, so we can change it locally without affecting entire radio 
    // if it was the selected theme
    this->theme = theme;
    auto colorList = theme->getColorList();
    themeColorList.assign(colorList.begin(), colorList.end());
    colorListWindow->setColorList(themeColorList);
    colorEntryEditor->setColorToEdit(colorListWindow->getSelectedColor());
    invalidate();
  }

 protected:
  std::vector<ColorEntry> themeColorList;
  ThemeFile *theme;
  ColorList *colorListWindow = nullptr;
  ColorEntryEditor *colorEntryEditor = nullptr;
  std::function<void()> update;
};

class ListBox : public FormGroup
{
 public:
  ListBox(Window *parent, const rect_t &rect, std::vector<std::string> names,
          std::function<uint8_t()> getValue,
          std::function<void(uint8_t)> setValue, WindowFlags windowFlags = 0,
          LcdFlags lcdFlags = 0) :
      FormGroup(parent, rect, windowFlags),
      _getValue(std::move(getValue)),
      _setValue(std::move(setValue)),
      names(names)
  {
    setInnerHeight(names.size() * LINE_HEIGHT);
    setSelected(_getValue());
  }

  void setTitle(std::string title)
  {
    this->title = title;
  }

  void paint(BitmapBuffer *dc) override
  {
    uint32_t bgColor = 0;
    if (hasFocus()) {
      bgColor = COLOR_THEME_FOCUS;
    } else {
      bgColor = COLOR_THEME_PRIMARY2;
    }

    int curY = 0;
    int n = 0;
    for (auto name : names) {
      if (n == selected) {
        dc->drawSolidFilledRect(1, curY, rect.w - 2, LINE_HEIGHT, bgColor);
      }

      LcdFlags textColor =
          n == selected ? COLOR_THEME_SECONDARY2 : COLOR_THEME_PRIMARY1;

      dc->drawText(5, curY + 2, name.c_str(), textColor);
      curY += LINE_HEIGHT;
      n++;
    }
    if (!(windowFlags & (FORM_NO_BORDER | FORM_FORWARD_FOCUS))) {
      if (!editMode && hasFocus()) {
        dc->drawSolidRect(0, getScrollPositionY(), rect.w, rect.h, 2,
                          COLOR_THEME_FOCUS);
      } else if (!(windowFlags & FORM_BORDER_FOCUS_ONLY)) {
        dc->drawSolidRect(0, getScrollPositionY(), rect.w, rect.h, 1,
                          COLOR_THEME_SECONDARY2);
      }
    }
  }

  void setSelected(int selected)
  {
    if (selected != this->selected) {
      this->selected = selected;
      setScrollPositionY(LINE_HEIGHT * this->selected - LINE_HEIGHT);
      _setValue(this->selected);
      invalidate();
    }
  }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override
  {
    if (!isEditMode()) return FormField::onEvent(event);

    int oldSelected = selected;
    switch (event) {
      case EVT_KEY_FIRST(KEY_EXIT):
        setEditMode(false);
        changeEnd();
        onKeyPress();
        setFocus(SET_FOCUS_DEFAULT, this);
        break;

      case EVT_KEY_BREAK(KEY_ENTER):
        if (isEditMode()) changeEnd();
        setEditMode(!isEditMode());
        onKeyPress();
        break;
      case EVT_ROTARY_RIGHT:
        oldSelected = (selected + 1) % names.size();
        setSelected(oldSelected);
        onKeyPress();
        break;
      case EVT_ROTARY_LEFT:
        oldSelected--;
        if (oldSelected < 0) oldSelected = names.size() - 1;
        setSelected(oldSelected);
        onKeyPress();
        break;
      default:
        FormField::onEvent(event);
    }
  }
#endif

#if defined(HARDWARE_TOUCH)
  bool onTouchEnd(coord_t x, coord_t y) override
  {
    if (!isEnabled()) return false;

    if (!hasFocus()) {
      setFocus(SET_FOCUS_DEFAULT);
    }

    if (!isEditMode()) setEditMode(true);

    auto oldSelected = y / LINE_HEIGHT;
    setSelected(oldSelected);
    return true;
    ;
  }
#endif

 protected:
  std::string title;
  std::function<uint8_t()> _getValue;
  std::function<void(uint8_t)> _setValue;
  Window *innerWindow;
  std::vector<std::string> names;
  int selected = 0;
};

ThemeSetupPage::ThemeSetupPage() : PageTab("Themes", ICON_MODEL_NOTES) {}

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
          PreviewWindow *pw = static_cast<PreviewWindow *>(this->previewWindow);
          pw->setColorList(tp->getThemeByIndex(value)->getColorList());
          currentTheme = value;
        }
      });
  lb->setTitle("Themes");

  grid.spacer(LINE_HEIGHT * 3 + 5);
  r = grid.getLabelSlot();
  r.h += LINE_HEIGHT * 5;
  colorEditor = new ColorEditor(window, r, theme, 
    [=] () {
      PreviewWindow *pw = static_cast<PreviewWindow *>(this->previewWindow);
      ColorEditor *ce = static_cast<ColorEditor *>(this->colorEditor);
      pw->setColorList(ce->getColorList());
    },
    0, COLOR_THEME_PRIMARY1);

  r = grid.getFieldSlot();
  r.x += 5;
  r.w -= 5;
  r.y = 0;
  r.h = window->getRect().h;
  previewWindow =
      new PreviewWindow(window, r, theme->getColorList());

  grid.spacer(40);
  window->setInnerHeight(grid.getWindowHeight());
}
