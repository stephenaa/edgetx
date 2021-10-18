#include "color_editor.h"

#define SET_DIRTY() setDirty()

////////////////////////////////////////////////////////////////////////////////////////////////
/////                     ColorEditorContent
////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t HSVtoRGB(float H, float S,float V) 
{
  if (H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0) {
    return 0;
  }

  float s = S / 100;
  float v = V / 100;
  float C = s * v;
  float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
  float m = v-C;
  float r,g,b;

  if (H >= 0 && H < 60) {
    r = C, g = X, b = 0;
  }
  else if (H >= 60 && H < 120 ){
    r = X, g = C, b = 0;
  }
  else if (H >= 120 && H < 180) {
    r = 0, g = C, b = X;
  }
  else if (H >= 180 && H < 240) {
    r = 0, g = X, b = C;
  }
  else if (H >= 240 && H < 300) {
    r = X, g = 0, b = C;
  }
  else {
    r = C, g = 0, b = X;
  }
    int R = (r+m)*255;
    int G = (g+m)*255;
    int B = (b+m)*255;
    
    return RGB(R,G,B);
}

void RGBtoHSV(uint8_t R, uint8_t G, uint8_t B, float& fH, float& fS, float& fV) 
{
  float fR = (float)R / 255;
  float fG = (float)G / 255;
  float fB = (float)B / 255;
  float fCMax = max(max(fR, fG), fB);
  float fCMin = min(min(fR, fG), fB);
  float fDelta = fCMax - fCMin;
  
  if(fDelta > 0) {
    if(fCMax == fR) {
      fH = 60 * (fmod(((fG - fB) / fDelta), 6));
    } else if(fCMax == fG) {
      fH = 60 * (((fB - fR) / fDelta) + 2);
    } else if(fCMax == fB) {
      fH = 60 * (((fR - fG) / fDelta) + 4);
    }
    
    if(fCMax > 0) {
      fS = fDelta / fCMax;
    } else {
      fS = 0;
    }
    
    fV = fCMax;
  } else {
    fH = 0;
    fS = 0;
    fV = fCMax;
  }
  
  if(fH < 0) {
    fH = 360 + fH;
  }
}

ColorEditorContent::ColorEditorContent(ModalWindow *window, const rect_t rect, uint32_t color) :
  ModalWindowContent(window, rect)
{
  r = GET_RED(color); g = GET_GREEN(color); b = GET_BLUE(color);
  float h, s, v;
  RGBtoHSV(r, g, b, h, s, v);
  hue = (int) h;
  this->s = 100 * s;
  this->v = 100 * v;

  rect_t rText = {240, 60, 40, 20};
  rect_t rNumber = {290, 60, 40, 20};

  new StaticText(this, rText, "Red", 0, COLOR_THEME_PRIMARY1);
  rText.y += 23;
  rEdit = new NumberEdit(this, rNumber, 0, 255, [=] () { return r; }, [=] (int value) { r = value; });
  rNumber.y += 23;
  new StaticText(this, rText, "Green", 0, COLOR_THEME_PRIMARY1);
  rText.y += 23;
  gEdit = new NumberEdit(this, rNumber, 0, 255, [=] () { return g; }, [=] (int value) { g = value; });
  rNumber.y += 23;
  new StaticText(this, rText, "Blue", 0, COLOR_THEME_PRIMARY1);
  bEdit = new NumberEdit(this, rNumber, 0, 255, [=] () { return b; }, [=] (int value) { b = value; });

  invalidate();
}

#if defined(HARDWARE_TOUCH)
bool ColorEditorContent::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY)
{
  if (touchState.event == TE_SLIDE_END) {
    TRACE("touchState: %d", touchState.event);
    sliding = false;
    colorPicking = false;
    slidingWindow = nullptr;
    invalidate();
  } else if (sliding) {
    x -= 10;
    x = max(x, 0);
    x = min(x, 360);
    if (hue != x) {
      hue = x;
      slidingWindow = this;  // so we get the end slide message
      invalidate();
    }
  } else if (colorPicking) {
    s = min((x - 10) / 2, 100);
    s = max(s, 0);
    v = min(100 - ((y - 60)), 100);
    v = max(v, 0);
    
    int rgb = HSVtoRGB(hue, s, v);
    r = GET_RED(rgb);
    g = GET_GREEN(rgb);
    b = GET_BLUE(rgb);
    rEdit->setValue(r);
    gEdit->setValue(g);
    bEdit->setValue(b);
    invalidate();
  }

  return true;
}

bool ColorEditorContent::onTouchEnd(coord_t x, coord_t y)
{
  sliding = false;
  colorPicking = false;
  invalidate();
  return true;
}

bool ColorEditorContent::onTouchStart(coord_t x, coord_t y)
{
  if (! sliding && (y > 40 && y < 53))
  {
    hue = x - 10;
    hue = max(hue, 0);
    hue = min(hue, 360);
    sliding = true;
    
    invalidate();
  } else if (x >= 10 && x <= 212 && y >= 60 && y <= 161) {
    colorPicking = true;
    v = min(100 - (y - 60), 100);
    s = min((x - 10) / 2, 100);
    
    int rgb = HSVtoRGB(hue, s, v);
    r = GET_RED(rgb);
    g = GET_GREEN(rgb);
    b = GET_BLUE(rgb);
    rEdit->setValue(r);
    gEdit->setValue(g);
    bEdit->setValue(b);
    invalidate();
  }
  return true;
}
#endif

#if defined(HARDWARE_KEYS)
void ColorEditorContent::onEvent(event_t event)
{
  int oldSliderVal = hue;
  if (event == EVT_ROTARY_LEFT) {
    hue -= 2;
    killAllEvents();
  }
  else if (event == EVT_ROTARY_RIGHT) {
    hue += 2;
    killAllEvents();
  }

  hue = max(hue, 0);
  hue = min(hue, 360);
  if (hue != oldSliderVal) {
    invalidate();
  }
}
#endif

void ColorEditorContent::drawTop(BitmapBuffer *dc)
{
  int r = 255, g = 0, b = 0;
  int state = 0;
  for (int i = 0; i < 360; i++) {
    dc->drawSolidVerticalLine(i + 10, 35, 20, COLOR2FLAGS(RGB(r, g, b)));

    if (state == 0) {
      g = min(g + 5, 255);
      if (g == 255)
        state++;
    } else if (state == 1) {
      r = max(r - 5, 0);
      if (r == 0)
        state++;
    } else if (state == 2) {
      b = min(b + 5, 255);
      if (b == 255)
        state++;
    } else if (state == 3) {
      g = max(g - 5, 0);
      if (g == 0)
        state++;
    } else if (state == 4) {
      r = min(r + 5, 255);
      if (r == 255) 
        state++;
    } else if (state == 5) {
      b = max(b - 5, 0);
      if (b == 0)
        state++;
    }
  }

  dc->drawFilledCircle(hue + 10, 35 + (20 / 2), 5, COLOR2FLAGS(RGB(255,255,255)));
  if (sliding) {
    dc->drawText(hue + 5, 25, std::to_string(hue).c_str(), FONT(XXS) | COLOR2FLAGS(WHITE));
  }
}

void ColorEditorContent::drawGrid(BitmapBuffer *dc) 
{
  for (int s = 0; s <= 100; s++) {
    for (int v = 100; v >= 0; v--) {
      uint32_t rgbVal = HSVtoRGB(hue, s, v);
      dc->drawPixel((s * 2) + 10, ((100 - v)) + 60, rgbVal);
      dc->drawPixel((s * 2) + 11, ((100 - v)) + 60, rgbVal);
    }
  }

  dc->drawFilledCircle((this->s * 2) + 10, (100 - this->v) + 60, 3, COLOR2FLAGS(WHITE));
}

void ColorEditorContent::drawColor(BitmapBuffer *dc)
{
  int32_t rgb = RGB(r,g,b);

  dc->drawSolidFilledRect(240, 130, 90, 32, COLOR2FLAGS(rgb));
  dc->drawSolidRect(240, 130, 90, 32, 1, COLOR2FLAGS(BLACK));
}

void ColorEditorContent::paint(BitmapBuffer *dc)
{
  ModalWindowContent::paint(dc);
  drawTop(dc);
  drawGrid(dc);
  drawColor(dc);
}


////////////////////////////////////////////////////////////////////////////////////////////////
/////                     ColorEditorPopup
////////////////////////////////////////////////////////////////////////////////////////////////
ColorEditorPopup::ColorEditorPopup(Window *window, std::function<uint32_t ()> getValue, std::function<void (uint32_t value)> setValue) :
  ModalWindow(window, true),
  _getValue(getValue),
  _setValue(setValue)
{
  color = _getValue();
  rect_t r = { 50, 50, 360 + 20, 170 };
  content = new ColorEditorContent(this, r, color);
  content->setFocus();
  bringToTop();
  content->setTitle("Color Picker");
  killAllEvents();
}

#if defined(HARDWARE_TOUCH)
bool ColorEditorPopup::onTouchStart(coord_t x, coord_t y)
{
  content->onTouchStart(x - content->rect.x + content->scrollPositionX, y - content->rect.y + content->scrollPositionY);
  return true;
}

bool ColorEditorPopup::onTouchEnd(coord_t x, coord_t y)
{
  content->onTouchEnd(x - content->rect.x + content->scrollPositionX, y - content->rect.y + content->scrollPositionY);
  if (!Window::onTouchEnd(x, y) && closeWhenClickOutside) {
    onKeyPress();
    deleteLater();
  }
  return true;
}
#endif

void ColorEditorPopup::deleteLater(bool detach, bool trash)
{
  if (_deleted)
    return;

  Layer::pop(this);
  Window::deleteLater(detach, trash);
}


////////////////////////////////////////////////////////////////////////////////////////////////
/////                     ColorEntryEditor
////////////////////////////////////////////////////////////////////////////////////////////////
#define PREVIEW_Y_OFFSET 5
#define PREVIEW_HEIGHT 22
#define FIRST_EDIT (PREVIEW_HEIGHT + PREVIEW_Y_OFFSET + 3)
#define SECOND_EDIT (FIRST_EDIT + LINE_HEIGHT)
#define THIRD_EDIT (SECOND_EDIT + LINE_HEIGHT)

ColorEntryEditor::ColorEntryEditor(Window *window, rect_t rect, std::function<void(ColorEntry value)> setValue) : 
    FormGroup(window, rect),
    setValue(setValue)
{

  cSquare = new ColorSquare(this, {5, 5, 60, 20}, [=] (ColorEntry value) { });

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

void ColorEntryEditor::setDirty() { 
  invalidate();
  colorEntry.colorValue = RGB(r, g, b);
  cSquare->setColorToEdit(colorEntry);
  setValue(colorEntry);
}

void ColorEntryEditor::setColorToEdit(ColorEntry colorEntry)
{
  this->colorEntry = colorEntry;
  cSquare->setColorToEdit(colorEntry);
  r = GET_RED(colorEntry.colorValue);
  g = GET_GREEN(colorEntry.colorValue);
  b = GET_BLUE(colorEntry.colorValue);
  invalidate();
}


////////////////////////////////////////////////////////////////////////////////////////////////
/////                     ColorList
////////////////////////////////////////////////////////////////////////////////////////////////
ColorList::ColorList(
    Window *parent, const rect_t &rect, std::vector<ColorEntry> colors,
    std::function<void(uint32_t value)> setValue,
    WindowFlags windowFlag, LcdFlags lcdFlags) :
    ListBase(parent, rect, getColorListNames(colors), nullptr, setValue),
    colorList(colors),
    tp(ThemePersistance::instance())
{
  setSelected(0);
}

std::vector<std::string> ColorList::getColorListNames(std::vector<ColorEntry> colors)
{
  std::vector<std::string> names;
  char **colorNames = tp->getColorNames();
  for (auto color : colors) {
    names.emplace_back(colorNames[color.colorNumber]);
  }
  
  return names;
}

void ColorList::drawLine(BitmapBuffer *dc, const rect_t &rect, uint32_t index, LcdFlags lcdFlags)
{
  ListBase::drawLine(dc, rect, index, lcdFlags);
  dc->drawSolidFilledRect(rect.w - 22, rect.y, 16, lineHeight - 6,
                          COLOR2FLAGS(colorList[index].colorValue));
  dc->drawSolidRect(rect.w - 22, rect.y, 16, lineHeight - 6, 1, COLOR2FLAGS(BLACK));
}



////////////////////////////////////////////////////////////////////////////////////////////////
/////                     ColorEditor
////////////////////////////////////////////////////////////////////////////////////////////////
ColorEditor::ColorEditor(Window *window, rect_t rect, ThemeFile *theme, std::function<void()> update,
                         WindowFlags windowFlags, LcdFlags lcdFlags) :
    FormGroup(window, rect, windowFlags | NO_FOCUS), 
    themeColorList(theme->getColorList()),
    theme(theme),
    update(update)
{
  rect_t r = {rect.x + 10 + (rect.w * 3) / 5, rect.y + LINE_HEIGHT, ((rect.w * 2) / 5) - 12, rect.h - LINE_HEIGHT - 2};
  colorEntryEditor = new ColorEntryEditor(window, r, [=] (ColorEntry value) {
    auto found = std::find(this->themeColorList.begin(), this->themeColorList.end(), value);
    if (found != this->themeColorList.end()) {
      found->colorValue = value.colorValue;
      this->colorListWindow->setColorList(themeColorList);
      if (update != nullptr)
        update();
    }
  });

  r = {rect.x + 2, rect.y + LINE_HEIGHT + 2, (rect.w * 3) / 5 - 2,
              rect.h - LINE_HEIGHT - 4};
  colorListWindow = new ColorList(
      window, r, theme->getColorList(),
      [=](uint32_t value) {
        if (colorEntryEditor != nullptr) {
          colorEntryEditor->setColorToEdit(themeColorList[value]);
        }
      },
      windowFlags, lcdFlags);
}

void ColorEditor::paint(BitmapBuffer *dc)
{
  FormGroup::paint(dc);

  dc->drawSolidFilledRect(2, 2, rect.w - 4, LINE_HEIGHT, COLOR_THEME_SECONDARY2);
  dc->drawText(rect.w / 2, 4, "Theme Colors", COLOR_THEME_PRIMARY2 | CENTERED);
}

void ColorEditor::setTheme(ThemeFile *theme)
{
  // make a copy of the theme, so we can change it locally without affecting entire radio
  // if it was the selected them
  this->theme = theme;
  auto colorList = theme->getColorList();
  themeColorList.assign(colorList.begin(), colorList.end());
  colorListWindow->setColorList(themeColorList);
  colorEntryEditor->setColorToEdit(colorListWindow->getSelectedColor());
  invalidate();
}

