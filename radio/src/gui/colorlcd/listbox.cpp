#include "listbox.h"

constexpr int LONG_PRESS_10MS = 40;

extern inline tmr10ms_t getTicks()
{
  return g_tmr10ms;
}

ListBase::ListBase(Window *parent, const rect_t &rect, std::vector<std::string> names,
          std::function<uint32_t()> getValue,
          std::function<void(uint32_t)> setValue,
          uint8_t lineHeight,
          WindowFlags windowFlags, LcdFlags lcdFlags) :
  FormField(parent, rect, windowFlags),
  names(names),
  _getValue(std::move(getValue)),
  _setValue(std::move(setValue)),
  lineHeight(lineHeight)
{
#if defined(HARDWARE_TOUCH)
  duration10ms = 0;
#endif
  setInnerHeight(names.size() * lineHeight);
  if (_getValue != nullptr)
    setSelected(_getValue());
  else 
    setSelected(0);
}

void ListBase::setSelected(int selected)
{
  if (selected != this->selected && selected >= 0 && selected < names.size()) {
    this->selected = selected;
    setScrollPositionY(lineHeight * this->selected - lineHeight);
    _setValue(this->selected);
    invalidate();
  }
}

void ListBase::drawLine(BitmapBuffer *dc, const rect_t &rect, uint32_t index, LcdFlags lcdFlags)
{
  std::string name = names[index];
  dc->drawText(rect.x, rect.y, name.c_str(), lcdFlags);
}

void ListBase::paint(BitmapBuffer *dc)
{
  dc->clear(COLOR_THEME_SECONDARY3);
  uint32_t bgColor = 0;
  bgColor = hasFocus() ? COLOR_THEME_FOCUS : COLOR_THEME_PRIMARY2;

  int curY = 0;
  for (int n = 0; n < names.size(); n++) {
    if (n == selected) {
      dc->drawSolidFilledRect(1, curY, rect.w - 2, lineHeight, bgColor);
    }

    LcdFlags textColor =
        n == selected ? COLOR_THEME_SECONDARY2 : COLOR_THEME_PRIMARY1;

    drawLine(dc, { 5, curY + 2, rect.w, lineHeight}, n, textColor);

    curY += lineHeight;
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
  void ListBase::onEvent(event_t event)
  {
    if (!isEditMode()) 
      return FormField::onEvent(event);

    int oldSelected = selected;
    switch (event) {
      case EVT_KEY_FIRST(KEY_EXIT):
        if (isEditMode()) changeEnd();
        setEditMode(!isEditMode());
        onKeyPress();
        killAllEvents();
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
      case EVT_KEY_LONG(KEY_ENTER):
        if (longPressHandler) {
          killEvents(event);
          longPressHandler(event);
        }
        break;

      default:
        FormField::onEvent(event);
    }
  }
#endif

#if defined(HARDWARE_TOUCH)
bool ListBase::isLongPress()
{
  unsigned int curTimer = getTicks();
  return (slidingWindow == nullptr && duration10ms != 0 && curTimer - duration10ms > LONG_PRESS_10MS);
}


void ListBase::checkEvents(void)
{
  Window::checkEvents();

  if (isLongPress()) {
    if (longPressHandler) {
      setSelected(yDown / lineHeight);
      longPressHandler(0);
      killAllEvents();
      duration10ms = 0;
      return;
    }
  }
}

bool ListBase::onTouchStart(coord_t x, coord_t y)
{
  if (duration10ms == 0) {
    duration10ms = getTicks();
  }

  yDown = y;

  return true;  // stop the processing and say that i handled it i want the
}

bool ListBase::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  if (touchState.event == TE_SLIDE_END) { 
    duration10ms = 0;
  }
  
  return FormField::onTouchSlide(x, y, startX, startY, slideX, slideY);
}


bool ListBase::onTouchEnd(coord_t x, coord_t y)
{
  if (!isEnabled()) return false;

  auto selected = y / lineHeight;
  setSelected(selected);

  duration10ms = 0;

  if (!hasFocus()) {
    setFocus(SET_FOCUS_DEFAULT);
  }

  if (!isEditMode()) setEditMode(true);

  return true;
}
#endif


