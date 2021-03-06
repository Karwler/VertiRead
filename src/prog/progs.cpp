#include "progs.h"
#include "engine/drawSys.h"
#include "engine/fileSys.h"
#include "engine/inputSys.h"
#include "engine/scene.h"
#include "engine/world.h"

// PROGRAM TEXT

ProgState::Text::Text(string str, int height) :
	text(std::move(str)),
	length(World::drawSys()->textLength(text.c_str(), height) + Label::defaultTextMargin * 2)
{}

// PROGRAM STATE

void ProgState::eventEnter() {
	if (!World::program()->tryClosePopupThread() && World::scene()->select)
		World::scene()->select->onClick(World::scene()->select->position(), SDL_BUTTON_LEFT);
}

bool ProgState::eventCommonEscape() {
	if (World::scene()->getContext()) {
		World::scene()->setContext(nullptr);
		return true;
	}
	return World::program()->tryClosePopupThread();
}

void ProgState::eventSelect(Direction dir) {
	World::inputSys()->mouseLast = false;
	if (World::scene()->select && !dynamic_cast<Layout*>(World::scene()->select))
		World::scene()->select->onNavSelect(dir);
	else
		World::scene()->selectFirst();
}

void ProgState::eventUp() {
	eventSelect(Direction::up);
}

void ProgState::eventDown() {
	eventSelect(Direction::down);
}

void ProgState::eventLeft() {
	eventSelect(Direction::left);
}

void ProgState::eventRight() {
	eventSelect(Direction::right);
}

void ProgState::eventCursorUp(float amt) {
	World::winSys()->moveCursor(ivec2(-amt * cursorMoveFactor * World::winSys()->getDSec(), 0));
}

void ProgState::eventCursorDown(float amt) {
	World::winSys()->moveCursor(ivec2(amt * cursorMoveFactor * World::winSys()->getDSec(), 0));
}

void ProgState::eventCursorLeft(float amt) {
	World::winSys()->moveCursor(ivec2(0, -amt * cursorMoveFactor * World::winSys()->getDSec()));
}

void ProgState::eventCursorRight(float amt) {
	World::winSys()->moveCursor(ivec2(0, amt * cursorMoveFactor * World::winSys()->getDSec()));
}

void ProgState::eventFullscreen() {
	World::winSys()->setFullscreen(!World::sets()->fullscreen);
}

void ProgState::eventHide() {
	World::sets()->showHidden = !World::sets()->showHidden;
}

void ProgState::eventBoss() {
	World::winSys()->toggleOpacity();
}

void ProgState::eventRefresh() {
	World::scene()->resetLayouts();
}

void ProgState::onResize() {
	maxTooltipLength = World::drawSys()->viewport().w * 2 / 3;
}

uptr<Overlay> ProgState::createOverlay() {
	return nullptr;
}

uptr<Popup> ProgState::createPopupMessage(string msg, PCall ccal, string ctxt, Alignment malign) {
	Text ok(std::move(ctxt), popupLineHeight);
	Text mg(std::move(msg), popupLineHeight);
	Widget* first;
	vector<Widget*> bot = {
		new Widget(),
		first = new Label(ok.length, std::move(ok.text), ccal, nullptr, nullptr, nullptr, Alignment::center),
		new Widget()
	};
	vector<Widget*> con = {
		new Label(1.f, std::move(mg.text), nullptr, nullptr, nullptr, nullptr, malign),
		new Layout(1.f, std::move(bot), Direction::right, Layout::Select::none, 0)
	};
	return std::make_unique<Popup>(svec2(mg.length + Layout::defaultItemSpacing * 2, popupLineHeight * 2 + Layout::defaultItemSpacing * 3), std::move(con), first);
}

uptr<Popup> ProgState::createPopupChoice(string msg, PCall kcal, PCall ccal, Alignment malign) {
	Text mg(std::move(msg), popupLineHeight);
	Widget* first;
	vector<Widget*> bot = {
		first = new Label(1.f, "Yes", kcal, nullptr, nullptr, nullptr, Alignment::center),
		new Label(1.f, "No", ccal, nullptr, nullptr, nullptr, Alignment::center)
	};
	vector<Widget*> con = {
		new Label(1.f, std::move(mg.text), nullptr, nullptr, nullptr, nullptr, malign),
		new Layout(1.f, std::move(bot), Direction::right, Layout::Select::none, 0)
	};
	return std::make_unique<Popup>(svec2(mg.length + Layout::defaultItemSpacing * 2, popupLineHeight * 2 + Layout::defaultItemSpacing * 3), std::move(con), first);
}

uptr<Context> ProgState::createContext(vector<pair<string, PCall>>&& items, Widget* parent) {
	vector<Widget*> wgts(items.size());
	for (sizet i = 0; i < items.size(); ++i)
		wgts[i] = new Label(lineHeight, std::move(items[i].first), items[i].second, &Program::eventCloseContext);

	Widget* first = wgts[0];
	Rect rect = calcTextContextRect(wgts, mousePos(), ivec2(0, lineHeight));
	return std::make_unique<Context>(rect.pos(), rect.size(), vector<Widget*>{ new ScrollArea(1.f, std::move(wgts), Layout::defaultDirection, Layout::Select::none, 0) }, first, parent, Color::dark, nullptr, Layout::defaultDirection, contextMargin);
}

uptr<Context> ProgState::createComboContext(ComboBox* parent, PCall kcal) {
	ivec2 size = parent->size();
	vector<Widget*> wgts(parent->getOptions().size());
	for (sizet i = 0; i < parent->getOptions().size(); ++i)
		wgts[i] = new Label(size.y, parent->getOptions()[i], kcal, &Program::eventCloseContext);

	Widget* first = wgts[0];
	Rect rect = calcTextContextRect(wgts, parent->position(), size);
	return std::make_unique<Context>(rect.pos(), rect.size(), vector<Widget*>{ new ScrollArea(1.f, std::move(wgts), Layout::defaultDirection, Layout::Select::none, 0) }, first, parent, Color::dark, &Program::eventResizeComboContext, Layout::defaultDirection, contextMargin);
}

Rect ProgState::calcTextContextRect(const vector<Widget*>& items, ivec2 pos, ivec2 size, int margin) {
	for (Widget* it : items)
		if (Label* lbl = dynamic_cast<Label*>(it))
			if (int w = World::drawSys()->textLength(lbl->getText().c_str(), size.y) + lbl->getTextMargin() * 2 + Slider::barSize + margin * 2; w > size.x)
				size.x = w;
	size.y = size.y * int(items.size()) + margin * 2;

	ivec2 res = World::drawSys()->viewport().size();
	calcContextPos(pos.x, size.x, res.x);
	calcContextPos(pos.y, size.y, res.y);
	return Rect(pos, size);
}

void ProgState::calcContextPos(int& pos, int& siz, int limit) {
	if (siz < limit)
		pos = pos + siz <= limit ? pos : limit - siz;
	else {
		siz = limit;
		pos = 0;
	}
}

template <class T>
int ProgState::findMaxLength(T pos, T end, int height) {
	int width = 0;
	for (; pos != end; ++pos)
		if (int len = World::drawSys()->textLength(*pos, height) + Label::defaultTextMargin * 2; len > width)
			width = len;
	return width;
}

SDL_Texture* ProgState::makeTooltip(const char* str) {
	return World::drawSys()->renderText(str, tooltipHeight, maxTooltipLength);
}

SDL_Texture* ProgState::makeTooltipL(const char* str) {
	uint width = 0;
	for (const char* pos = str; *pos;) {
		sizet len = strcspn(pos, "\n");
		if (uint siz = World::drawSys()->textLength(string(pos, len).c_str(), tooltipHeight) + Label::tooltipMargin.x * 2; siz > width)
			if (width = std::min(siz, maxTooltipLength); width == maxTooltipLength)
				break;
		pos += pos[len] ? len + 1 : len;
	}
	return World::drawSys()->renderText(str, tooltipHeight, width);
}

// PROG BOOKS

void ProgBooks::eventEscape() {
	if (!eventCommonEscape())
		World::program()->eventTryExit();
}

void ProgBooks::eventHide() {
	ProgState::eventHide();
	World::scene()->resetLayouts();
}

void ProgBooks::eventFileDrop(const fs::path& file) {
	World::program()->openFile(file);
}

uptr<RootLayout> ProgBooks::createLayout() {
	// top bar
	Text download("Download", topHeight);
	Text settings("Settings", topHeight);
	Text exit("Exit", topHeight);
	vector<Widget*> top = {
#ifdef DOWNLOADER
		new Label(download.length, download.text, &Program::eventOpenDownloader),
#endif
		new Label(settings.length, std::move(settings.text), &Program::eventOpenSettings),
		new Label(exit.length, std::move(exit.text), &Program::eventTryExit)
	};

	// book list
	vector<fs::path> books = FileSys::listDir(World::sets()->getDirLib(), false, true, World::sets()->showHidden);
	vector<Widget*> tiles(books.size()+1);
	for (sizet i = 0; i < books.size(); ++i) {
		Text txt(books[i].u8string(), TileBox::defaultItemHeight);
		tiles[i] = new Label(txt.length, std::move(txt.text), &Program::eventOpenPageBrowser, &Program::eventOpenBookContext);
	}
	tiles.back() = new Button(TileBox::defaultItemHeight, &Program::eventOpenPageBrowser, &Program::eventOpenBookContext, nullptr, makeTooltip("Browse other directories"), true, World::drawSys()->texture("search"));

	// root layout
	vector<Widget*> cont = {
		new Layout(topHeight, std::move(top), Direction::right, Layout::Select::none, topSpacing),
		new TileBox(1.f, std::move(tiles))
	};
	return std::make_unique<RootLayout>(1.f, std::move(cont), Direction::down, Layout::Select::none, topSpacing);
}

// PROG PAGE BROWSER

void ProgPageBrowser::eventEscape() {
	if (!eventCommonEscape())
		World::program()->eventBrowserGoUp();
}

void ProgPageBrowser::eventHide() {
	ProgState::eventHide();
	World::scene()->resetLayouts();
}

void ProgPageBrowser::eventFileDrop(const fs::path& file) {
	World::program()->openFile(file);
}

uptr<RootLayout> ProgPageBrowser::createLayout() {
	// sidebar
	initlist<const char*> txs = {
		"Exit",
		"Up"
	};
	initlist<const char*>::iterator itxs = txs.begin();
	int txsWidth = findMaxLength(txs.begin(), txs.end(), lineHeight);
	vector<Widget*> bar = {
		new Label(lineHeight, *itxs++, &Program::eventExitBrowser),
		new Label(lineHeight, *itxs++, &Program::eventBrowserGoUp)
	};

	// list of files and directories
	auto [files, dirs] = World::browser()->listCurDir(World::sets()->showHidden);
	vector<Widget*> items(files.size() + dirs.size());
	for (sizet i = 0; i < dirs.size(); ++i)
		items[i] = new Label(lineHeight, std::move(dirs[i]), &Program::eventBrowserGoIn, nullptr, nullptr, nullptr, Alignment::left, World::drawSys()->texture("folder"));
	for (sizet i = 0; i < files.size(); ++i)
		items[dirs.size()+i] = new Label(lineHeight, std::move(files[i]), &Program::eventBrowserGoFile, nullptr, nullptr, nullptr, Alignment::left, World::drawSys()->texture("file"));

	// main content
	vector<Widget*> mid = {
		new Layout(txsWidth, std::move(bar)),
		new ScrollArea(1.f, std::move(items))
	};

	// root layout
	vector<Widget*> cont = {
		new LabelEdit(lineHeight, World::browser()->getRootDir() != Browser::topDir ? relativePath(World::browser()->getCurDir(), World::sets()->getDirLib()).u8string() : World::browser()->getCurDir().u8string(), &Program::eventBrowserGoTo),
		new Layout(1.f, std::move(mid), Direction::right, Layout::Select::none, topSpacing)
	};
	return std::make_unique<RootLayout>(1.f, std::move(cont), Direction::down, Layout::Select::none, topSpacing);
}

// PROG READER

void ProgReader::eventEscape() {
	if (!eventCommonEscape())
		World::program()->eventExitReader();
}

void ProgReader::eventUp() {
	eventScrollUp(1.f);
}

void ProgReader::eventDown() {
	eventScrollDown(1.f);
}

void ProgReader::eventLeft() {
	eventScrollLeft(1.f);
}

void ProgReader::eventRight() {
	eventScrollRight(1.f);
}

void ProgReader::eventScrollUp(float amt) {
	reader->onScroll(ivec2(0, -modifySpeed(amt * World::sets()->scrollSpeed.y)));
}

void ProgReader::eventScrollDown(float amt) {
	reader->onScroll(ivec2(0, modifySpeed(amt * World::sets()->scrollSpeed.y)));
}

void ProgReader::eventScrollRight(float amt) {
	reader->onScroll(ivec2(modifySpeed(amt * World::sets()->scrollSpeed.x), 0));
}

void ProgReader::eventScrollLeft(float amt) {
	reader->onScroll(ivec2(-modifySpeed(amt * World::sets()->scrollSpeed.x), 0));
}

void ProgReader::eventNextPage() {
	if (!reader->scrollToNext())
		World::program()->eventNextDir();
}

void ProgReader::eventPrevPage() {
	if (!reader->scrollToPrevious())
		World::program()->eventPrevDir();
}

void ProgReader::eventZoomIn() {
	World::program()->eventZoomIn();
}

void ProgReader::eventZoomOut() {
	World::program()->eventZoomOut();
}

void ProgReader::eventZoomReset() {
	World::program()->eventZoomReset();
}

void ProgReader::eventToStart() {
	reader->scrollToLimit(true);
}

void ProgReader::eventToEnd() {
	reader->scrollToLimit(false);
}

void ProgReader::eventCenterView() {
	World::program()->eventCenterView();
}

void ProgReader::eventNextDir() {
	World::program()->eventNextDir();
}

void ProgReader::eventPrevDir() {
	World::program()->eventPrevDir();
}

void ProgReader::eventHide() {
	ProgState::eventHide();
	World::program()->eventStartLoadingReader(reader->firstPage());
}

void ProgReader::eventClosing() {
	if (fs::path rpath = relativePath(World::browser()->getCurDir(), World::sets()->getDirLib()); rpath.empty())
		World::fileSys()->saveLastPage(dotStr, World::browser()->getCurDir().u8string(), reader->curPage());
	else {
		string path = rpath.u8string();
		string::iterator mid = std::find_if(path.begin(), path.end(), isDsep);
		string::iterator nxt = std::find_if(mid, path.end(), notDsep);
		World::fileSys()->saveLastPage(string(path.begin(), mid), string(nxt, path.end()), reader->curPage());
	}
	SDL_ShowCursor(SDL_ENABLE);
}

uptr<RootLayout> ProgReader::createLayout() {
	vector<Widget*> cont = { reader = new ReaderBox(1.f, {}, World::sets()->direction, World::sets()->zoom, World::sets()->spacing) };
	return std::make_unique<RootLayout>(1.f, std::move(cont), Direction::right, Layout::Select::none, 0);
}

uptr<Overlay> ProgReader::createOverlay() {
	vector<Widget*> menu = {
		new Button(picSize, &Program::eventExitReader, nullptr, nullptr, makeTooltip("Exit"), false, World::drawSys()->texture("cross"), picMargin),
		new Button(picSize, &Program::eventNextDir, nullptr, nullptr, makeTooltip("Next"), false, World::drawSys()->texture("right"), picMargin),
		new Button(picSize, &Program::eventPrevDir, nullptr, nullptr, makeTooltip("Previous"), false, World::drawSys()->texture("left"), picMargin),
		new Button(picSize, &Program::eventZoomIn, nullptr, nullptr, makeTooltip("Zoom in"), false, World::drawSys()->texture("plus"), picMargin),
		new Button(picSize, &Program::eventZoomOut, nullptr, nullptr, makeTooltip("Zoom out"), false, World::drawSys()->texture("minus"), picMargin),
		new Button(picSize, &Program::eventZoomReset, nullptr, nullptr, makeTooltip("Zoom reset"), false, World::drawSys()->texture("reset"), picMargin),
		new Button(picSize, &Program::eventCenterView, nullptr, nullptr, makeTooltip("Center"), false, World::drawSys()->texture("center"), picMargin)
	};
	int ysiz = picSize * int(menu.size());
	return std::make_unique<Overlay>(svec2(0), svec2(picSize, ysiz), svec2(0), svec2(picSize/2, ysiz), std::move(menu), Color::normal, Direction::down, 0);
}

int ProgReader::modifySpeed(float value) {
	if (float factor = 1.f; World::inputSys()->isPressed(Binding::Type::scrollFast, factor))
		value *= scrollFactor * factor;
	else if (World::inputSys()->isPressed(Binding::Type::scrollSlow, factor))
		value /= scrollFactor * factor;
	return int(value * World::winSys()->getDSec());
}

// PROG DOWNLOADER

#ifdef DOWNLOADER
void ProgDownloader::eventEscape() {
	if (!eventCommonEscape())
		World::program()->eventOpenBookList();
}

uptr<RootLayout> ProgDownloader::createLayout() {
	// top bar
	vector<string> wsrcs(WebSource::sourceNames.begin(), WebSource::sourceNames.end());
	Text books("Library", topHeight);
	Text settings("Settings", topHeight);
	Text exit("Exit", topHeight);
	Text downloads("Downloads", topHeight);
	vector<Widget*> top = {
		new Label(books.length, std::move(books.text), &Program::eventOpenBookList),
		new Label(settings.length, std::move(settings.text), &Program::eventOpenSettings),
		new Label(exit.length, std::move(exit.text), &Program::eventTryExit),
		new Widget(),
		new Label(downloads.length, std::move(downloads.text), &Program::eventOpenDownloadList),
		new ComboBox(findMaxLength(wsrcs.begin(), wsrcs.end(), topHeight), World::downloader()->getSource()->name(), std::move(wsrcs), &Program::eventSwitchSource, nullptr, Alignment::center)
	};

	// download button and bar
	Text download("Download all", lineHeight);
	vector<Widget*> dow = {
		chaptersTick = new CheckBox(lineHeight, false, &Program::eventSelectAllChapters, &Program::eventSelectAllChapters),
		new Label(download.length, std::move(download.text), &Program::eventDownloadAllChapters)
	};

	// info and chapter section (left side children)
	vector<Widget*> inf = {
		new Layout(lineHeight, std::move(dow), Direction::right),
		chapters = new ScrollArea()
	};

	// search bar children
	Text search("Search", lineHeight);
	vector<Widget*> nav = {
		query = new LabelEdit(0.8f, string(), &Program::eventQuery, nullptr, nullptr, nullptr, LabelEdit::TextType::text, false),
		new Label(search.length, std::move(search.text), &Program::eventQuery)
	};

	// search bar + query results (right side)
	vector<Widget*> qrs = {
		new Layout(lineHeight, std::move(nav), Direction::right, Layout::Select::none, topSpacing),
		results = new ScrollArea(1.f, {}, Direction::down, Layout::Select::one)
	};

	// left + right
	vector<Widget*> mid = {
		new Layout(1.f, std::move(inf), Direction::down, Layout::Select::none),
		new Layout(2.f, std::move(qrs), Direction::down)
	};

	// root layout
	vector<Widget*> cont = {
		new Layout(topHeight, std::move(top), Direction::right, Layout::Select::none, topSpacing),
		new Layout(1.f, std::move(mid), Direction::right, Layout::Select::none, topSpacing)
	};
	return std::make_unique<RootLayout>(1.f, std::move(cont), Direction::down, Layout::Select::none, topSpacing);
}

Comic ProgDownloader::curInfo() const {
	Comic info(static_cast<LabelEdit*>(*results->getSelected().begin())->getText(), vector<pairStr>(chapterUrls.size()));
	for (sizet i = 0; i < chapterUrls.size(); ++i)
		info.chapters[i] = pair(static_cast<Label*>(static_cast<Layout*>(chapters->getWidget(i))->getWidget(1))->getText(), chapterUrls[i]);
	return info;
}

void ProgDownloader::printResults(vector<pairStr>&& comics) {
	resultUrls.resize(comics.size());
	vector<Widget*> wgts(comics.size());
	for (sizet i = 0; i < wgts.size(); ++i) {
		wgts[i] = new Label(TileBox::defaultItemHeight, std::move(comics[i].first), &Program::eventShowComicInfo, nullptr, &Program::eventDownloadComic);
		resultUrls[i] = std::move(comics[i].second);
	}
	results->setWidgets(std::move(wgts));
}

void ProgDownloader::printInfo(vector<pairStr>&& chaps) {
	chapterUrls.resize(chaps.size());
	vector<Widget*> wgts(chaps.size());
	for (sizet i = 0; i < wgts.size(); ++i) {
		vector<Widget*> line = {
			new CheckBox(TileBox::defaultItemHeight, true),
			new Label(1.f, std::move(chaps[i].first), &Program::eventSelectChapter, nullptr, &Program::eventDownloadChapter)
		};
		wgts[i] = new Layout(TileBox::defaultItemHeight, std::move(line), Direction::right);
		chapterUrls[i] = std::move(chaps[i].second);
	}
	chapters->setWidgets(std::move(wgts));
	chaptersTick->on = !chaps.empty();
}

// PROG DOWNLOADS

void ProgDownloads::eventEscape() {
	if (!eventCommonEscape())
		World::program()->eventOpenDownloader();
}

uptr<RootLayout> ProgDownloads::createLayout() {
	// top bar
	Text books("Library", topHeight);
	Text download("Download", topHeight);
	Text settings("Settings", topHeight);
	Text exit("Exit", topHeight);
	vector<Widget*> top = {
		new Label(books.length, std::move(books.text), &Program::eventOpenBookList),
		new Label(download.length, std::move(download.text), &Program::eventOpenDownloader),
		new Label(settings.length, std::move(settings.text), &Program::eventOpenSettings),
		new Label(exit.length, std::move(exit.text), &Program::eventTryExit)
	};

	// sidebar
	initlist<const char*> txs = {
		"Resume",
		"Stop",
		"Clear"
	};
	initlist<const char*>::iterator itxs = txs.begin();
	int txsWidth = findMaxLength(txs.begin(), txs.end(), lineHeight);
	vector<Widget*> bar = {
		new Label(lineHeight, *itxs++, &Program::eventResumeDownloads),
		new Label(lineHeight, *itxs++, &Program::eventStopDownloads),
		new Label(lineHeight, *itxs++, &Program::eventClearDownloads)
	};

	// queue list
	World::downloader()->queueLock.lock();
	vector<Widget*> lse(World::downloader()->getDlQueue().size());
	for (sizet i = 0; i < lse.size(); ++i) {
		vector<Widget*> ln = {
			new Label(1.f, World::downloader()->getDlQueue()[i].title + " - waiting"),
			new Label(TileBox::defaultItemHeight, "X", &Program::eventDownloadDelete)
		};
		lse[i] = new Layout(TileBox::defaultItemHeight, std::move(ln), Direction::right);
	}
	if (!lse.empty())
		static_cast<Label*>(static_cast<Layout*>(lse[0])->getWidget(0))->setText(World::downloader()->getDlQueue()[0].title + " - " + to_string(World::downloader()->getDlProg().x) + '/' + to_string(World::downloader()->getDlProg().y));
	World::downloader()->queueLock.unlock();

	// main content
	vector<Widget*> mid = {
		new Layout(txsWidth, std::move(bar)),
		list = new ScrollArea(1.f, std::move(lse))
	};

	// root layout
	vector<Widget*> cont = {
		new Layout(topHeight, std::move(top), Direction::right, Layout::Select::none, topSpacing),
		new Layout(1.f, std::move(mid), Direction::right)
	};
	return std::make_unique<RootLayout>(1.f, std::move(cont), Direction::down, Layout::Select::none, topSpacing);
}
#endif

// PROG SETTINGS

void ProgSettings::eventEscape() {
	if (!eventCommonEscape())
		World::program()->eventOpenBookList();
}

void ProgSettings::eventFullscreen() {
	ProgState::eventFullscreen();
	fullscreen->on = World::sets()->fullscreen;
}

void ProgSettings::eventHide() {
	ProgState::eventHide();
	showHidden->on = World::sets()->showHidden;
}

void ProgSettings::eventFileDrop(const fs::path& file) {
	if (FileSys::isFont(file)) {
		World::drawSys()->setFont(file.u8string(), World::sets(), World::fileSys());
		World::scene()->resetLayouts();
	} else {
		try {
			if (fs::is_directory(file))
				World::sets()->setDirLib(file, World::fileSys()->getDirSets());
		} catch (const std::runtime_error& err) {
			std::cerr << err.what() << std::endl;
		}
	}
}

uptr<RootLayout> ProgSettings::createLayout() {
	// top bar
	Text books("Library", topHeight);
	Text download("Download", topHeight);
	Text exit("Exit", topHeight);
	vector<Widget*> top = {
		new Label(books.length, std::move(books.text), &Program::eventOpenBookList),
#ifdef DOWNLOADER
		new Label(download.length, std::move(download.text), &Program::eventOpenDownloader),
#endif
		new Label(exit.length, std::move(exit.text), &Program::eventTryExit)
	};

	// setting buttons and labels
	Text tcnt("Count:", lineHeight);
	Text tsiz("Size:", lineHeight);
	Text tsizp("Portrait", lineHeight);
	Text tsizl("Landscape", lineHeight);
	Text tsizs("Square", lineHeight);
	Text tsizf("Fill", lineHeight);
	initlist<const char*> txs = {
		"Direction",
		"Zoom",
		"Spacing",
		"Picture limit",
		"Fullscreen",
		"Size",
		"Theme",
		"Show hidden",
		"Font",
		"Library",
		"Renderer",
		"Scroll speed",
		"Deadzone"
	};
	initlist<const char*>::iterator itxs = txs.begin();
	array<string, Binding::names.size()> bnames;
	for (sizet i = 0; i < Binding::names.size(); ++i)
		bnames[i] = firstUpper(Binding::names[i]);
	int descLength = std::max(findMaxLength(txs.begin(), txs.end(), lineHeight), findMaxLength(bnames.begin(), bnames.end(), lineHeight));

	constexpr char tipPicLim[] = "Picture limit per batch:\n"
		"- none: all pictures in directory/archive\n"
		"- count: number of pictures\n"
		"- size: total size of pictures";
	constexpr char tipDeadzon[] = "Controller axis deadzone";

	// action fields for labels
	SDL_RendererInfo rendererInfo;
	vector<string> renderers((sizet(SDL_GetNumRenderDrivers())));
	for (sizet i = 0; i < renderers.size(); ++i)
		if (!SDL_GetRenderDriverInfo(int(i), &rendererInfo))
			renderers[i] = rendererInfo.name;
	vector<string> themes = World::fileSys()->getAvailableThemes();
	vector<string> directs(Direction::names.begin(), Direction::names.end());
	vector<string> plims(PicLim::names.begin(), PicLim::names.end());
	Text dots(KeyGetter::ellipsisStr, lineHeight);
	Text dznum(to_string(Settings::axisLimit), lineHeight);
	vector<Widget*> lx[] = { {
		new Label(descLength, *itxs++),
		new ComboBox(1.f, directs[uint8(World::sets()->direction)], std::move(directs), &Program::eventSwitchDirection, makeTooltip("Reading direction"))
	}, {
		new Label(descLength, *itxs++),
		new LabelEdit(1.f, trimZero(to_string(World::sets()->zoom)), &Program::eventSetZoom, nullptr, nullptr, makeTooltip("Default reader zoom"), LabelEdit::TextType::uFloat)
	}, {
		new Label(descLength, *itxs++),
		new LabelEdit(1.f, to_string(World::sets()->spacing), &Program::eventSetSpacing, nullptr, nullptr, makeTooltip("Picture spacing in reader"), LabelEdit::TextType::uInt)
	}, {
		new Label(descLength, *itxs++),
		new ComboBox(findMaxLength(plims.begin(), plims.end(), lineHeight), plims[uint8(World::sets()->picLim.type)], std::move(plims), &Program::eventSetPicLimitType, makeTooltipL(tipPicLim)),
		createLimitEdit()
	}, {
		new Label(descLength, *itxs++),
		fullscreen = new CheckBox(lineHeight, World::sets()->fullscreen, &Program::eventSetFullscreen, nullptr, nullptr, makeTooltip("Fullscreen in native resolution"))
	}, {
		new Label(descLength, *itxs++),
		new Label(tsizp.length, std::move(tsizp.text), &Program::eventSetPortrait, nullptr, nullptr, makeTooltip("Portrait window size")),
		new Label(tsizl.length, std::move(tsizl.text), &Program::eventSetLandscape, nullptr, nullptr, makeTooltip("Landscape window size")),
		new Label(tsizs.length, std::move(tsizs.text), &Program::eventSetSquare, nullptr, nullptr, makeTooltip("Square window size")),
		new Label(tsizf.length, std::move(tsizf.text), &Program::eventSetFill, nullptr, nullptr, makeTooltip("Fill screen with window"))
	}, {
		new Label(descLength, *itxs++),
		new ComboBox(1.f, World::sets()->getTheme(), std::move(themes), &Program::eventSetTheme, makeTooltip("Color scheme"))
	}, {
		new Label(descLength, *itxs++),
		showHidden = new CheckBox(lineHeight, World::sets()->showHidden, &Program::eventSetHide, nullptr, nullptr, makeTooltip("Show hidden files"))
	}, {
		new Label(descLength, *itxs++),
		new LabelEdit(1.f, World::sets()->font, &Program::eventSetFont, nullptr, nullptr, makeTooltip("Font name or path"))
	}, {
		new Label(descLength, *itxs++),
		new LabelEdit(1.f, World::sets()->getDirLib().u8string(), &Program::eventSetLibraryDirLE, nullptr, nullptr, makeTooltip("Library path")),
		new Label(dots.length, std::move(dots.text), &Program::eventOpenLibDirBrowser, nullptr, nullptr, makeTooltip("Browse for library"), Alignment::center)
	}, {
		new Label(descLength, *itxs++),
		new ComboBox(1.f, World::sets()->renderer, std::move(renderers), &Program::eventSetRenderer, makeTooltip("Graphics renderer"))
	}, {
		new Label(descLength, *itxs++),
		new LabelEdit(1.f, World::sets()->scrollSpeedString(), &Program::eventSetScrollSpeed, nullptr, nullptr, makeTooltip("Scroll speed for button presses or axes"), LabelEdit::TextType::sFloatSpaced)
	}, {
		new Label(descLength, *itxs++),
		deadzoneSL = new Slider(1.f, World::sets()->getDeadzone(), 0, Settings::axisLimit, &Program::eventSetDeadzoneSL, nullptr, nullptr, makeTooltip(tipDeadzon)),
		deadzoneLE = new LabelEdit(dznum.length + LabelEdit::caretWidth, to_string(World::sets()->getDeadzone()), &Program::eventSetDeadzoneLE, nullptr, nullptr, makeTooltip(tipDeadzon), LabelEdit::TextType::uInt)
	} };
	sizet lcnt = txs.size();
	vector<Widget*> lns(lcnt + 1 + bnames.size() + 2);
	for (sizet i = 0; i < lcnt; ++i)
		lns[i] = new Layout(lineHeight, std::move(lx[i]), Direction::right);
	lns[lcnt] = new Widget(0);
	limitLine = static_cast<Layout*>(lns[3]);

	// shortcut entries
	for (sizet i = 0; i < bnames.size(); ++i) {
		Label* lbl = new Label(descLength, std::move(bnames[i]));
		vector<Widget*> lin {
			lbl,
			new KeyGetter(1.f, KeyGetter::AcceptType::keyboard, Binding::Type(i), makeTooltip((lbl->getText() + " keyboard binding").c_str())),
			new KeyGetter(1.f, KeyGetter::AcceptType::joystick, Binding::Type(i), makeTooltip((lbl->getText() + " joystick binding").c_str())),
			new KeyGetter(1.f, KeyGetter::AcceptType::gamepad, Binding::Type(i), makeTooltip((lbl->getText() + " gamepad binding").c_str()))
		};
		lns[lcnt+1+i] = new Layout(lineHeight, std::move(lin), Direction::right);
	}
	lcnt += 1 + Binding::names.size();

	// reset button
	Text resbut("Reset", lineHeight);
	lns[lcnt] = new Widget(0);
	lns[lcnt+1] = new Layout(lineHeight, { new Label(resbut.length, std::move(resbut.text), &Program::eventResetSettings, nullptr, nullptr, makeTooltip("Reset all settings")) }, Direction::right);

	// root layout
	vector<Widget*> cont = {
		new Layout(topHeight, std::move(top), Direction::right, Layout::Select::none, topSpacing),
		new ScrollArea(1.f, std::move(lns))
	};
	return std::make_unique<RootLayout>(1.f, std::move(cont), Direction::down, Layout::Select::none, topSpacing);
}

Widget* ProgSettings::createLimitEdit() {
	switch (World::sets()->picLim.type) {
	case PicLim::Type::count:
		return new LabelEdit(1.f, to_string(World::sets()->picLim.getCount()), &Program::eventSetPicLimCount, nullptr, nullptr, makeTooltip("Number of pictures per batch"), LabelEdit::TextType::uInt);
	case PicLim::Type::size:
		return new LabelEdit(1.f, PicLim::memoryString(World::sets()->picLim.getSize()), &Program::eventSetPicLimSize, nullptr, nullptr, makeTooltip("Total size of pictures per batch"));
	}
	return new Widget();
}

// PROG SEARCH DIR

void ProgSearchDir::eventEscape() {
	if (!eventCommonEscape())
		World::program()->eventBrowserGoUp();
}

void ProgSearchDir::eventHide() {
	ProgState::eventHide();
	World::scene()->resetLayouts();
}

uptr<RootLayout> ProgSearchDir::createLayout() {
	// sidebar
	initlist<const char*> txs = {
		"Exit",
		"Up",
		"Set"
	};
	initlist<const char*>::iterator itxs = txs.begin();
	int txsWidth = findMaxLength(txs.begin(), txs.end(), lineHeight);
	vector<Widget*> bar = {
		new Label(lineHeight, *itxs++, &Program::eventExitBrowser),
		new Label(lineHeight, *itxs++, &Program::eventBrowserGoUp),
		new Label(lineHeight, *itxs++, &Program::eventSetLibraryDirBW)
	};

	// directory list
	vector<fs::path> strs = FileSys::listDir(World::browser()->getCurDir(), false, true, World::sets()->showHidden);
	vector<Widget*> items(strs.size());
	for (sizet i = 0; i < strs.size(); ++i)
		items[i] = new Label(lineHeight, strs[i].u8string(), nullptr, nullptr, &Program::eventBrowserGoIn, nullptr, Alignment::left, World::drawSys()->texture("folder"));

	// main content
	vector<Widget*> mid = {
		new Layout(txsWidth, std::move(bar)),
		list = new ScrollArea(1.f, std::move(items), Direction::down, Layout::Select::one)
	};

	// root layout
	vector<Widget*> cont = {
		new LabelEdit(lineHeight, World::browser()->getCurDir().u8string(), &Program::eventBrowserGoTo),
		new Layout(1.f, std::move(mid), Direction::right, Layout::Select::none, topSpacing)
	};
	return std::make_unique<RootLayout>(1.f, std::move(cont), Direction::down, Layout::Select::none, topSpacing);
}
