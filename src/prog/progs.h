#pragma once

#include "downloader.h"
#include "utils/settings.h"

// for handling program state specific things that occur in all states
class ProgState {
public:
	static constexpr char dotStr[] = ".";

protected:
	struct Text {
		string text;
		int length;

		Text(string str, int height);
	};

	static constexpr int popupLineHeight = 40;
	static constexpr int tooltipHeight = 16;
	static constexpr int lineHeight = 30;
	static constexpr int topHeight = 40;
	static constexpr int topSpacing = 10;
	static constexpr int picSize = 40;
	static constexpr int picMargin = 4;
	static constexpr int contextMargin = 3;
private:
	static constexpr float cursorMoveFactor = 10.f;

	uint maxTooltipLength;

public:
	ProgState();
	virtual ~ProgState() = default;	// to keep the compiler happy

	void eventEnter();
	virtual void eventEscape() {}
	virtual void eventUp();
	virtual void eventDown();
	virtual void eventLeft();
	virtual void eventRight();
	virtual void eventScrollUp(float) {}
	virtual void eventScrollDown(float) {}
	virtual void eventScrollLeft(float) {}
	virtual void eventScrollRight(float) {}
	void eventCursorUp(float amt);
	void eventCursorDown(float amt);
	void eventCursorLeft(float amt);
	void eventCursorRight(float amt);
	virtual void eventCenterView() {}
	virtual void eventNextPage() {}
	virtual void eventPrevPage() {}
	virtual void eventZoomIn() {}
	virtual void eventZoomOut() {}
	virtual void eventZoomReset() {}
	virtual void eventToStart() {}
	virtual void eventToEnd() {}
	virtual void eventNextDir() {}
	virtual void eventPrevDir() {}
	virtual void eventFullscreen();
	virtual void eventHide();
	void eventBoss();
	void eventRefresh();
	virtual void eventFileDrop(const fs::path&) {}
	virtual void eventClosing() {}
	void onResize();

	virtual uptr<RootLayout> createLayout() = 0;
	virtual uptr<Overlay> createOverlay();
	static uptr<Popup> createPopupMessage(string msg, PCall ccal, string ctxt = "Okay", Alignment malign = Alignment::left);
	static uptr<Popup> createPopupChoice(string msg, PCall kcal, PCall ccal, Alignment malign = Alignment::left);
	static uptr<Context> createContext(vector<pair<string, PCall>>&& items, Widget* parent);
	static uptr<Context> createComboContext(ComboBox* parent, PCall kcal);

	static Rect calcTextContextRect(const vector<Widget*>& items, ivec2 pos, ivec2 size, int margin = contextMargin);
protected:
	template <class T> static int findMaxLength(T pos, T end, int height);
	SDL_Texture* makeTooltip(const char* str);
	SDL_Texture* makeTooltipL(const char* str);

	bool eventCommonEscape();	// returns true if something happened
private:
	void eventSelect(Direction dir);
	static void calcContextPos(int& pos, int& siz, int limit);
};

inline ProgState::ProgState() {
	onResize();
}

class ProgBooks : public ProgState {
public:
	~ProgBooks() final = default;

	void eventEscape() final;
	void eventHide() final;
	void eventFileDrop(const fs::path& file) final;

	uptr<RootLayout> createLayout() final;
};

class ProgPageBrowser : public ProgState {
public:
	~ProgPageBrowser() final = default;

	void eventEscape() final;
	void eventHide() final;
	void eventFileDrop(const fs::path& file) final;

	uptr<RootLayout> createLayout() final;
};

class ProgReader : public ProgState {
public:
	ReaderBox* reader;
private:
	static constexpr float scrollFactor = 2.f;

public:
	~ProgReader() final = default;

	void eventEscape() final;
	void eventUp() final;
	void eventDown() final;
	void eventLeft() final;
	void eventRight() final;
	void eventScrollUp(float amt) final;
	void eventScrollDown(float amt) final;
	void eventScrollLeft(float amt) final;
	void eventScrollRight(float amt) final;
	void eventCenterView() final;
	void eventNextPage() final;
	void eventPrevPage() final;
	void eventZoomIn() final;
	void eventZoomOut() final;
	void eventZoomReset() final;
	void eventToStart() final;
	void eventToEnd() final;
	void eventNextDir() final;
	void eventPrevDir() final;
	void eventHide() final;
	void eventClosing() final;

	uptr<RootLayout> createLayout() final;
	uptr<Overlay> createOverlay() final;

private:
	static int modifySpeed(float value);	// change scroll speed depending on pressed bindings
};

#ifdef DOWNLOADER
class ProgDownloader : public ProgState {
public:
	LabelEdit* query;
	ScrollArea* results;
	ScrollArea* chapters;
	CheckBox* chaptersTick;

	vector<string> resultUrls, chapterUrls;

public:
	~ProgDownloader() final = default;

	void eventEscape() final;

	uptr<RootLayout> createLayout() final;
	Comic curInfo() const;
	void printResults(vector<pairStr>&& comics);
	void printInfo(vector<pairStr>&& chaps);
};

class ProgDownloads : public ProgState {
public:
	ScrollArea* list;

public:
	~ProgDownloads() final = default;

	void eventEscape() final;

	uptr<RootLayout> createLayout() final;
};
#endif

class ProgSettings : public ProgState {
public:
	fs::path oldPathBuffer;	// for keeping old library path between decisions
	Layout* limitLine;
	Slider* deadzoneSL;
	LabelEdit* deadzoneLE;
private:
	CheckBox* fullscreen;
	CheckBox* showHidden;

public:
	~ProgSettings() final = default;

	void eventEscape() final;
	void eventFullscreen() final;
	void eventHide() final;
	void eventFileDrop(const fs::path& file) final;

	uptr<RootLayout> createLayout() final;

	Widget* createLimitEdit();
};

class ProgSearchDir : public ProgState {
public:
	ScrollArea* list;

	~ProgSearchDir() final = default;

	void eventEscape() final;
	void eventHide() final;

	uptr<RootLayout> createLayout() final;
};
