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

#define LEFT_LIST_WIDTH 200
#define MARGIN_WIDTH 5


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

constexpr int BUTTON_HEIGHT = 30;
constexpr int BUTTON_WIDTH  = 75;

class ThemeDetailsDialog: public Dialog
{
  public:
    ThemeDetailsDialog(Window *parent, ThemeFile theme, std::function<void (ThemeFile theme)> saveHandler = nullptr) :
      Dialog(parent, "Edit Details", {50, 50, 400, 170}),
      theme(theme),
      saveHandler(saveHandler)
    {
      FormGridLayout grid(400);
      grid.setLabelWidth(150);
      grid.spacer(8);

      new StaticText(&content->form, grid.getLabelSlot(), "Name", 0, COLOR_THEME_PRIMARY1);
      new TextEdit(&content->form, grid.getFieldSlot(), this->theme.getName(), NAME_LENGTH);
      grid.nextLine();
      new StaticText(&content->form, grid.getLabelSlot(), "Author", 0, COLOR_THEME_PRIMARY1);
      new TextEdit(&content->form, grid.getFieldSlot(), this->theme.getAuthor(), AUTHOR_LENGTH);
      grid.nextLine();
      new StaticText(&content->form, grid.getLineSlot(), "Description", 0, COLOR_THEME_PRIMARY1);
      grid.nextLine();
      new TextEdit(&content->form, grid.getLineSlot(), this->theme.getInfo(), INFO_LENGTH);
      grid.nextLine();

      rect_t r = {400 - (BUTTON_WIDTH + 5), grid.getWindowHeight() + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
      new TextButton(&content->form, r, "Save", [=] () {
        if (saveHandler != nullptr)
          saveHandler(this->theme);
        deleteLater();
        return 0;
      });
      r.x -= (BUTTON_WIDTH + 5);
      new TextButton(&content->form, r, "Cancel", [=] () {
        deleteLater();
        return 0;
      });
    }
  protected:
    ThemeFile theme;
    std::function<void(ThemeFile theme)> saveHandler = nullptr;
};

class ThemeEditPage : public Page
{
  public:
    ThemeEditPage(ThemeFile theme, std::function<void (ThemeFile &theme)> saveHandler = nullptr) :
      Page(ICON_MODEL_NOTES),
      theme(theme),
      page(this),
      saveHandler(std::move(saveHandler))
    {
      buildBody(&body);
      buildHeader(&header);
    }

    void buildHeader(FormGroup *window)
    {
      // page title
      new StaticText(window,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
                     "Edit Theme", 0, COLOR_THEME_PRIMARY2);
      themeName = new StaticText(window,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                      LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                     theme.getName(), 0, COLOR_THEME_PRIMARY2);

      // save and cancel
      rect_t r = {LCD_W - (BUTTON_WIDTH + 5), 4, BUTTON_WIDTH, BUTTON_HEIGHT };
      saveButton = new TextButton(window, r, "Save", [=] () {
        if (saveHandler != nullptr)
          saveHandler(this->theme);
        deleteLater();
        return 0;
      });
      r.x -= (BUTTON_WIDTH + 5);
      detailButton = new TextButton(window, r, "Details", [=] () {
        new ThemeDetailsDialog(page, theme, [=] (ThemeFile t) {
          theme.setAuthor(t.getAuthor());
          theme.setInfo(t.getInfo());
          theme.setName(t.getName());

          // update the theme name
          themeName->setText(theme.getName());
        });
        return 0;
      });

      // setup the prev next controls so save and details are in the mix
      window->link(cList, detailButton);
      window->link(saveButton, cList);
    }

    void buildBody(FormGroup *window)
    {
      rect_t r = { 0, 4, LEFT_LIST_WIDTH, LCD_H - TOPBAR_HEIGHT };
      cList = new ColorList(window, r, theme.getColorList(), 
        [=] (uint32_t value) {
          if (previewWindow) {
            theme.setColorByIndex(value, cList->getSelectedColor().colorValue);
            colorMaintainer.setColorList(theme.getColorList());
            previewWindow->invalidate();
          }
        });
        
      r = { LEFT_LIST_WIDTH + MARGIN_WIDTH, 4, LCD_W - LEFT_LIST_WIDTH - MARGIN_WIDTH, LCD_H - TOPBAR_HEIGHT };
      previewWindow = new PreviewWindow(window, r, theme.getColorList());
    }

  protected:
    ThemeFile theme;
    Page *page;
    std::function<void(ThemeFile &theme)> saveHandler = nullptr;
    PreviewWindow *previewWindow = nullptr;
    ColorList *cList = nullptr;
    StaticText *themeName = nullptr;
    TextButton *saveButton;
    TextButton *detailButton;
};


ThemeSetupPage::ThemeSetupPage() : PageTab("Theme Editor", ICON_MODEL_NOTES) {}

void ThemeSetupPage::build(FormWindow *window)
{
  auto tp = ThemePersistance::instance();
  auto theme = tp->getCurrentTheme();
  currentTheme = tp->getThemeIndex();

  rect_t r = { 0, 4, LEFT_LIST_WIDTH, LCD_H - TOPBAR_HEIGHT - 38 };

  listBox = new ListBox(
    window, r, tp->getNames(),
      [=]() { 
        return currentTheme;
      },
      [=](uint8_t value) {
        if (themeColorPreview && authorText && nameText && filePreview) {
          ThemeFile *theme = tp->getThemeByIndex(value);
          themeColorPreview->setColorList(theme->getColorList());
          authorText->setText(theme->getAuthor());
          nameText->setText(theme->getName());
          filePreview->setFile(theme->getThemeImageFileName().c_str());
        }
        currentTheme = value;
      });
  listBox->setTitle("Themes");
  listBox->setLongPressHandler([=] (event_t event) {
    auto menu = new Menu(window,false);

    // you cant edit the default theme
    if (listBox->getSelected() != 0) {
      menu->addLine("Edit Theme",
        [=] () {
          new ThemeEditPage(*tp->getThemeByIndex(currentTheme), [=](ThemeFile &theme) {
            auto t = tp->getThemeByIndex(currentTheme);
            if (t != nullptr) {
              t->setName(theme.getName());
              t->setAuthor(theme.getAuthor());
              t->setInfo(theme.getInfo());
              int n = 0;

              // update the colors that were edited
              for (auto color : theme.getColorList()) {
                t->setColorByIndex(n, color.colorValue);
                n++;
              }
            }
          });
      });
    }
    menu->addLine("New", [=] () {});
    menu->addLine("Set Active", [=] () {
      tp->applyTheme(listBox->getSelected());
      tp->setDefaultTheme(listBox->getSelected());
    });
    if (listBox->getSelected() != 0) {
      menu->addLine("Delete", [=] () {
        TRACE("SELECTED");
      });
    }
  });

  r.x = LEFT_LIST_WIDTH + MARGIN_WIDTH;
  r.w = LCD_W - r.x;

  r.h = 20;
  themeColorPreview = new ThemeColorPreview(window, r, theme->getColorList());
  r.y += 24;

  r.h = 150;
  filePreview = new FilePreview(window, r, false);
  filePreview->setFile(theme->getThemeImageFileName().c_str());

  r.h = 22;
  r.y += 130;
  new StaticText(window, r, "Name:");
  r.x += 80;
  nameText = new StaticText(window, r, theme->getName());

  r.y += 22;
  r.x -= 80;
  new StaticText(window, r, "Author:");
  r.x += 80;
  authorText = new StaticText(window, r, theme->getAuthor());

  // window->setInnerHeight(grid.getWindowHeight());
}
