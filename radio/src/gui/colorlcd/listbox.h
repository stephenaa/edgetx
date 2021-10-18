#pragma once
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include "bitmapbuffer.h"
#include "libopenui.h"

// base class for lists of elements with names
class ListBase : public FormField
{
  public:
    ListBase(Window *parent, const rect_t &rect, std::vector<std::string> names,
              std::function<uint32_t()> getValue,
              std::function<void(uint32_t)> setValue, 
              uint8_t lineHeight = LINE_HEIGHT,
              WindowFlags windowFlags = 0, LcdFlags lcdFlags = 0);
    void paint (BitmapBuffer *dc) override;
    
    // draw one line of the list. Default implementation just draws the names.  Oher types of lists
    // can draw anything they want.
    virtual void drawLine(BitmapBuffer *dc, const rect_t &rect, uint32_t index, LcdFlags lcdFlags);
    virtual void setSelected(int selected);

    void setLongPressHandler(std::function<void(event_t)> handler)
    {
      longPressHandler = handler;
    }

    inline void setLineHeight(uint8_t height)
    {
      lineHeight = height;
    }

    inline void setNames(std::vector<std::string> names)
    {
      this->names.assign(names.begin(), names.end());
      setInnerHeight(names.size() * lineHeight);
      invalidate();
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ListBox";
    }
#endif

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    void checkEvents(void) override;
    bool onTouchEnd(coord_t x, coord_t y) override;
    bool onTouchStart(coord_t x, coord_t y) override;
    bool isLongPress();
#endif

  protected:
    std::function<void(event_t)> longPressHandler = nullptr;
    std::vector<std::string> names;
    std::function<uint32_t()> _getValue;
    std::function<void(uint32_t)> _setValue;
    int lineHeight;
    int32_t selected = -1;
    
#if defined(HARDWARE_TOUCH)
    uint32_t duration10ms;
    bool longPressed = false;
#endif
};


class ListBox : public ListBase
{
 public:
  using ListBase::ListBase;

  inline void setTitle(std::string title)
  {
    this->title = title;
  }

 protected:
  std::string title;
};

