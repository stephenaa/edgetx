#include <algorithm>
#include "radio_theme.h"
#include "opentx.h"
#include "view_main.h"
#include "widget_settings.h"
#include "libopenui.h"
#include "theme_manager.h"
#include "file_preview.h"

class ListBox : public FormField
{
  public:
    ListBox(Window *parent, const rect_t &rect, std::vector<std::string> names, std::function<uint8_t()> getValue, std::function<void(uint8_t)> setValue, WindowFlags flags = 0) :
      FormField(parent, rect, flags),
      _getValue(std::move(getValue)),
      _setValue(std::move(setValue)),
      names(names)
    {
    }

#define LINE_HEIGHT PAGE_LINE_HEIGHT
    void paint(BitmapBuffer * dc) override
    {
        if (!(windowFlags & (FORM_NO_BORDER | FORM_FORWARD_FOCUS))) {
            if (!editMode && hasFocus()) {
            dc->drawSolidRect(0, 0, rect.w, rect.h, 2, COLOR_THEME_FOCUS);
            }
            else if (!(windowFlags & FORM_BORDER_FOCUS_ONLY)) {
            dc->drawSolidRect(0, 0, rect.w, rect.h, 1, COLOR_THEME_SECONDARY2);
            }
        }

        int curY = 0;
        for (auto name: names) {
            dc->drawText(5, curY, name.c_str(), COLOR_THEME_SECONDARY1);
            curY += LINE_HEIGHT;
        }

        this->setInnerHeight(curY);
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override
    {
    }
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override
    {
        return true;
    }
#endif

  protected:
    std::function<uint8_t()> _getValue;
    std::function<void(uint8_t)> _setValue;
    std::vector<std::string> names;
};


ThemeSetupPage::ThemeSetupPage():
  PageTab("Themes", ICON_MODEL_NOTES)
{
}

void ThemeSetupPage::build(FormWindow * window)
{
    FormGridLayout grid;

    std::vector<std::string> names;
    for (int i = 0; i < 30; i++) {
        names.emplace_back("Theme Number : " + std::to_string(i));
    }

    rect_t r = grid.getLabelSlot();
    r.h += LINE_HEIGHT * 5;
    new ListBox(window, r, names, [=] (){
        return 0;
    },
    [=] (uint8_t value) {
    });
}

