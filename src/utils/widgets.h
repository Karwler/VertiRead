#pragma once

#include "settings.h"

// size of a widget in pixels or relative to it's parent
struct Size {
	union {
		int pix;	// use if type is pix
		float prc;	// use if type is prc
	};
	bool usePix;

	Size(int pixels);
	Size(float percent = 1.f);

	void set(int pixels);
	void set(float percent);
};
using svec2 = glm::vec<2, Size, glm::defaultp>;

inline Size::Size(int pixels) :
	pix(pixels),
	usePix(true)
{}

inline Size::Size(float percent) :
	prc(percent),
	usePix(false)
{}

inline void Size::set(int pixels) {
	usePix = true;
	pix = pixels;
}

inline void Size::set(float percent) {
	usePix = false;
	prc = percent;
}

// can be used as spacer
class Widget {
protected:
	Layout* parent = nullptr;	// every widget that isn't a Layout should have a parent
	sizet index = SIZE_MAX;		// this widget's id in parent's widget list
	Size relSize;				// size relative to parent's parameters

public:
	Widget(const Size& size = Size());
	virtual ~Widget() = default;

	virtual void drawSelf() const {}	// calls appropriate drawing function(s) in DrawSys
	virtual void onResize() {}	// for updating values when window size changed
	virtual void tick(float) {}
	virtual void postInit() {}	// gets called after parent is set and all set up
	virtual void onClick(ivec2, uint8) {}
	virtual void onDoubleClick(ivec2, uint8) {}
	virtual void onMouseMove(ivec2, ivec2) {}
	virtual void onHold(ivec2, uint8) {}
	virtual void onDrag(ivec2, ivec2) {}	// mouse move while left button down
	virtual void onUndrag(uint8) {}			// gets called on mouse button up if instance is Scene's capture
	virtual void onScroll(ivec2) {}	// on mouse wheel y movement
	virtual void onKeypress(const SDL_Keysym&) {}
	virtual void onJButton(uint8) {}
	virtual void onJHat(uint8, uint8) {}
	virtual void onJAxis(uint8, bool) {}
	virtual void onGButton(SDL_GameControllerButton) {}
	virtual void onGAxis(SDL_GameControllerAxis, bool) {}
	virtual void onText(const char*) {}
	virtual void onNavSelect(Direction dir);
	virtual bool navSelectable() const;
	virtual bool hasDoubleclick() const;

	sizet getIndex() const;
	Layout* getParent() const;
	void setParent(Layout* pnt, sizet id);

	const Size& getRelSize() const;
	virtual ivec2 position() const;
	virtual ivec2 size() const;
	ivec2 center() const;
	Rect rect() const;			// the rectangle that is the widget
	virtual Rect frame() const;	// the rectangle to restrain a widget's visibility (in Widget it returns the parent's frame and if in Layout, it returns a frame for it's children)
};

inline Widget::Widget(const Size& size) :
	relSize(size)
{}

inline sizet Widget::getIndex() const {
	return index;
}

inline Layout* Widget::getParent() const {
	return parent;
}

inline const Size& Widget::getRelSize() const {
	return relSize;
}

inline ivec2 Widget::center() const {
	return position() + size() / 2;
}

inline Rect Widget::rect() const {
	return Rect(position(), size());
}

// visible widget with texture and background color
class Picture : public Widget {
public:
	static constexpr int defaultIconMargin = 2;

	SDL_Texture* tex;	// doesn't get freed automatically
	bool showBG;
protected:
	int texMargin;

public:
	Picture(const Size& size = Size(), bool bg = true, SDL_Texture* texture = nullptr, int margin = defaultIconMargin);
	~Picture() override = default;

	void drawSelf() const override;

	virtual Color color() const;
	virtual Rect texRect() const;
};

// clickable widget with function calls for left and right click (it's rect is drawn so you can use it like a spacer with color)
class Button : public Picture {
public:
	static constexpr ivec2 tooltipMargin = { 4, 1 };

protected:
	PCall lcall, rcall, dcall;
private:
	SDL_Texture* tooltip;

public:
	Button(const Size& size = Size(), PCall leftCall = nullptr, PCall rightCall = nullptr, PCall doubleCall = nullptr, SDL_Texture* tip = nullptr, bool bg = true, SDL_Texture* texture = nullptr, int margin = defaultIconMargin);
	~Button() override;

	void onClick(ivec2 mPos, uint8 mBut) override;
	void onDoubleClick(ivec2 mPos, uint8 mBut) override;
	bool navSelectable() const override;
	bool hasDoubleclick() const override;

	Color color() const override;
	SDL_Texture* getTooltip();
	Rect tooltipRect(ivec2& tres) const;
};

inline SDL_Texture* Button::getTooltip() {
	return tooltip;
}

// if you don't know what a checkbox is then I don't know what to tell ya
class CheckBox : public Button {
public:
	bool on;

	CheckBox(const Size& size = Size(), bool checked = false, PCall leftCall = nullptr, PCall rightCall = nullptr, PCall doubleCall = nullptr, SDL_Texture* tip = nullptr, bool bg = true, SDL_Texture* texture = nullptr, int margin = defaultIconMargin);
	~CheckBox() final = default;

	void drawSelf() const final;
	void onClick(ivec2 mPos, uint8 mBut) final;

	Rect boxRect() const;
	Color boxColor() const;
	bool toggle();
};

inline Color CheckBox::boxColor() const {
	return on ? Color::light : Color::dark;
}

inline bool CheckBox::toggle() {
	return on = !on;
}

// horizontal slider (maybe one day it'll be able to be vertical)
class Slider : public Button {
public:
	static constexpr int barSize = 10;
private:
	int val, vmin, vmax;
	int diffSliderMouse = 0;

public:
	Slider(const Size& size = Size(), int value = 0, int minimum = 0, int maximum = 255, PCall leftCall = nullptr, PCall rightCall = nullptr, PCall doubleCall = nullptr, SDL_Texture* tip = nullptr, bool bg = true, SDL_Texture* texture = nullptr, int margin = defaultIconMargin);
	~Slider() final = default;

	void drawSelf() const final;
	void onClick(ivec2 mPos, uint8 mBut) final;
	void onHold(ivec2 mPos, uint8 mBut) final;
	void onDrag(ivec2 mPos, ivec2 mMov) final;
	void onUndrag(uint8 mBut) final;

	int getVal() const;
	void setVal(int value);

	Rect barRect() const;
	Rect sliderRect() const;

private:
	void setSlider(int xpos);
	int sliderPos() const;
	int sliderLim() const;
};

inline int Slider::getVal() const {
	return val;
}

inline void Slider::setVal(int value) {
	val = std::clamp(value, vmin, vmax);
}

inline int Slider::sliderPos() const {
	return position().x + size().y/4 + val * sliderLim() / vmax;
}

// horizontal progress bar
class ProgressBar : public Picture {
private:
	static constexpr int barMarginFactor = 8;

	int val, vmin, vmax;

public:
	ProgressBar(const Size& size = Size(), int value = 0, int minimum = 0, int maximum = 255, bool bg = true, SDL_Texture* texture = nullptr, int margin = defaultIconMargin);
	~ProgressBar() final = default;

	void drawSelf() const final;

	int getVal() const;
	void setVal(int value);

	Rect barRect() const;
};

inline int ProgressBar::getVal() const {
	return val;
}

inline void ProgressBar::setVal(int value) {
	val = std::clamp(value, vmin, vmax);
}

// it's a little ass backwards but labels (aka a line of text) are buttons
class Label : public Button {
public:
	static constexpr int defaultTextMargin = 5;

	SDL_Texture* textTex = nullptr;
protected:
	string text;
	int textMargin;
	Alignment align;	// text alignment

public:
	Label(const Size& size = Size(), string line = string(), PCall leftCall = nullptr, PCall rightCall = nullptr, PCall doubleCall = nullptr, SDL_Texture* tip = nullptr, Alignment alignment = Alignment::left, SDL_Texture* texture = nullptr, bool bg = true, int lineMargin = defaultTextMargin, int iconMargin = defaultIconMargin);
	~Label() override;

	void drawSelf() const override;
	void onResize() override;
	void postInit() override;

	const string& getText() const;
	virtual void setText(const string& str);
	virtual void setText(string&& str);
	Rect textRect() const;
	Rect textFrame() const;
	Rect texRect() const override;
	int textIconOffset() const;
	int getTextMargin() const;
protected:
	virtual ivec2 textPos() const;
	virtual void updateTextTex();
};

inline const string& Label::getText() const {
	return text;
}

inline Rect Label::textRect() const {
	return Rect(textPos(), texSize(textTex));
}

inline int Label::getTextMargin() const {
	return textMargin;
}

// for switching between multiple options (kinda like a drop-down menu except I was too lazy to make an actual one)
class ComboBox : public Label {
private:
	vector<string> options;
	sizet curOpt;

public:
	ComboBox(const Size& size = Size(), string curOption = string(), vector<string>&& opts = vector<string>(), PCall call = nullptr, SDL_Texture* tip = nullptr, Alignment alignment = Alignment::left, SDL_Texture* texture = nullptr, bool bg = true, int lineMargin = defaultTextMargin, int iconMargin = defaultIconMargin);
	~ComboBox() final = default;

	void onClick(ivec2 mPos, uint8 mBut) final;

	const vector<string>& getOptions() const;
	sizet getCurOpt() const;
	void setCurOpt(sizet id);
};

inline const vector<string>& ComboBox::getOptions() const {
	return options;
}

inline sizet ComboBox::getCurOpt() const {
	return curOpt;
}

// for editing a line of text (ignores Label's align), (calls Button's lcall on text confirm rather than on click)
class LabelEdit : public Label {
public:
	enum class TextType : uint8 {
		text,
		sInt,
		sIntSpaced,
		uInt,
		uIntSpaced,
		sFloat,
		sFloatSpaced,
		uFloat,
		uFloatSpaced
	};

	static constexpr int caretWidth = 4;

	bool unfocusConfirm;
private:
	TextType textType;
	int textOfs = 0;	// text's horizontal offset
	uint cpos = 0;		// caret position
	string oldText;

public:
	LabelEdit(const Size& size = Size(), string line = string(), PCall leftCall = nullptr, PCall rightCall = nullptr, PCall doubleCall = nullptr, SDL_Texture* tip = nullptr, TextType type = TextType::text, bool focusLossConfirm = true, SDL_Texture* texture = nullptr, bool bg = true, int lineMargin = defaultTextMargin, int iconMargin = defaultIconMargin);
	~LabelEdit() final = default;

	void onClick(ivec2 mPos, uint8 mBut) final;
	void onKeypress(const SDL_Keysym& key) final;
	void onText(const char* str) final;

	const string& getOldText() const;
	void setText(const string& str) final;
	void setText(string&& str) final;
	Rect caretRect() const;

	void confirm();
	void cancel();

private:
	void onTextReset();
	ivec2 textPos() const final;
	int caretPos() const;	// caret's relative x position
	void setCPos(uint cp);

	static bool kmodCtrl(uint16 mod);
	static bool kmodAlt(uint16 mod);
	uint jumpCharB(uint i);
	uint jumpCharF(uint i);
	uint findWordStart();	// returns index of first character of word before cpos
	uint findWordEnd();		// returns index of character after last character of word after cpos
	void cleanText();
	void cleanSIntSpacedText();
	void cleanUIntSpacedText();
	void cleanSFloatText();
	void cleanSFloatSpacedText();
	void cleanUFloatText();
	void cleanUFloatSpacedText();
};

inline const string& LabelEdit::getOldText() const {
	return oldText;
}

inline bool LabelEdit::kmodCtrl(uint16 mod) {
	return mod & KMOD_CTRL && !(mod & (KMOD_SHIFT | KMOD_ALT));
}

inline bool LabelEdit::kmodAlt(uint16 mod) {
	return mod & KMOD_ALT && !(mod & (KMOD_SHIFT | KMOD_CTRL));
}

// for getting a key/button/axis
class KeyGetter : public Label {
public:
	enum class AcceptType : uint8 {
		keyboard,
		joystick,
		gamepad
	};

	static constexpr char ellipsisStr[] = "...";
private:
	static constexpr char prefButton[] = "B ";
	static constexpr char prefHat[] = "H ";
	static constexpr char prefSep = ' ';
	static constexpr char prefAxis[] = "A ";
	static constexpr char prefAxisPos = '+';
	static constexpr char prefAxisNeg = '-';

	AcceptType acceptType;		// what kind of binding is being accepted
	Binding::Type bindingType;	// index of what is currently being edited

public:
	KeyGetter(const Size& size = Size(), AcceptType type = AcceptType::keyboard, Binding::Type binding = Binding::Type(-1), SDL_Texture* tip = nullptr, Alignment alignment = Alignment::center, SDL_Texture* texture = nullptr, bool bg = true, int lineMargin = defaultTextMargin, int iconMargin = defaultIconMargin);
	~KeyGetter() final = default;

	void onClick(ivec2 mPos, uint8 mBut) final;
	void onKeypress(const SDL_Keysym& key) final;
	void onJButton(uint8 jbutton) final;
	void onJHat(uint8 jhat, uint8 value) final;
	void onJAxis(uint8 jaxis, bool positive) final;
	void onGButton(SDL_GameControllerButton gbutton) final;
	void onGAxis(SDL_GameControllerAxis gaxis, bool positive) final;
	bool navSelectable() const final;

	void restoreText();
	void clearBinding();
private:
	static string bindingText(Binding::Type binding, AcceptType accept);
};

inline void KeyGetter::restoreText() {
	setText(bindingText(bindingType, acceptType));
}
