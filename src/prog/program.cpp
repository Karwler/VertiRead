#include "engine/world.h"

// BOOKS

Program::Program() :
	state(new ProgState)	// necessary as a placeholder to prevent nullptr exceptions
{}

void Program::init() {
	if (!(World::getArgs().size() && openFile(World::getArg(0))))
		eventOpenBookList();
}

void Program::eventOpenBookList(Button* but) {
	setState(new ProgBooks);
}

void Program::eventOpenPageBrowser(Button* but) {
	browser.reset(new Browser(dynamic_cast<Label*>(but) ? appendDsep(World::winSys()->sets.getDirLib()) + static_cast<Label*>(but)->getText() : "", "", &Program::eventOpenBookList));
	setState(new ProgPageBrowser);
}

void Program::eventOpenReader(Button* but) {
	startReader(static_cast<Label*>(but)->getText());
}

void Program::eventOpenLastPage(Button* but) {
	Label* lbl = dynamic_cast<Label*>(but);
	string file = Filer::getLastPage(lbl ? lbl->getText() : ".");
	if (file.empty())
		eventOpenPageBrowser(but);
	else {
		browser.reset(new Browser(lbl ? appendDsep(World::winSys()->sets.getDirLib()) + lbl->getText() : "", parentPath(file), &Program::eventOpenBookList));
		if (!startReader(filename(file)))
			eventOpenPageBrowser(but);
	}
}

bool Program::openFile(string file) {
	file = getAbsolute(file);
	if (Filer::isPicture(file)) {
		browser.reset(new Browser(string(1, dsep), parentPath(file), &Program::eventOpenBookList));
		if (startReader(filename(file)))
			return true;
		browser.reset();
	} else if (Filer::fileType(file) == FTYPE_DIR) {
		browser.reset(new Browser(string(1, dsep), file, &Program::eventOpenBookList));
		setState(new ProgPageBrowser);
		return true;
	}
	return false;
}

// BROWSER

void Program::eventBrowserGoUp(Button* but) {
	if (browser->goUp())
		World::scene()->resetLayouts();
	else
		eventExitBrowser();
}

void Program::eventBrowserGoIn(Button* but) {
	if (browser->goIn(static_cast<Label*>(but)->getText()))
		World::scene()->resetLayouts();
}

void Program::eventExitBrowser(Button* but) {
	void (Program::*call)(Button*) = browser->exCall;
	browser.reset();
	(this->*call)(nullptr);
}

// READER

bool Program::startReader(const string& picname) {
	if (!browser->selectPicture(picname))
		return false;

	setState(new ProgReader);
	return true;
}

void Program::eventZoomIn(Button* but) {
	static_cast<ReaderBox*>(World::scene()->getLayout())->setZoom(Default::zoomFactor);
}

void Program::eventZoomOut(Button* but) {
	static_cast<ReaderBox*>(World::scene()->getLayout())->setZoom(1.f / Default::zoomFactor);
}

void Program::eventZoomReset(Button* but) {
	ReaderBox* reader = static_cast<ReaderBox*>(World::scene()->getLayout());
	reader->setZoom(1.f / reader->getZoom());
}

void Program::eventCenterView(Button* but) {
	static_cast<ReaderBox*>(World::scene()->getLayout())->centerListX();
}

void Program::eventNextDir(Button* but) {
	browser->goNext();
	browser->selectFirstPicture();
	World::scene()->resetLayouts();
}

void Program::eventPrevDir(Button* but) {
	browser->goPrev();
	browser->selectFirstPicture();
	World::scene()->resetLayouts();
}

void Program::eventExitReader(Button* but) {
	SDL_ShowCursor(SDL_ENABLE);
	setState(new ProgPageBrowser);
}

// SETTINGS

void Program::eventOpenSettings(Button* but) {
	setState(new ProgSettings);
}

void Program::eventSwitchLanguage(Button* but) {
	World::drawSys()->setLanguage(static_cast<SwitchBox*>(but)->getText());
	World::scene()->resetLayouts();
}

void Program::eventSetLibraryDirLE(Button* but) {
	LineEdit* le = static_cast<LineEdit*>(but);
	if (World::winSys()->sets.setDirLib(le->getText()) != le->getText()) {
		World::scene()->setPopup(ProgState::createPopupMessage("Invalid directory."));
		le->setText(World::winSys()->sets.getDirLib());
	}
}

void Program::eventSetLibraryDirBW(Button* but) {
	const uset<Widget*>& select = static_cast<Layout*>(World::scene()->getLayout()->getWidget(1))->getSelected();
	string path = select.size() ? appendDsep(browser->getCurDir()) + static_cast<Label*>(*select.begin())->getText() : browser->getCurDir();

	World::winSys()->sets.setDirLib(path);
	browser.reset();
	eventOpenSettings();
}

void Program::eventOpenLibDirBrowser(Button* but) {
#ifdef _WIN32
	browser.reset(new Browser("\\", std::getenv("UserProfile"), &Program::eventOpenSettings));
#else
	browser.reset(new Browser("/", std::getenv("HOME"), &Program::eventOpenSettings));
#endif
	setState(new ProgSearchDir);
}

void Program::eventSwitchFullscreen(Button* but) {
	World::winSys()->setFullscreen(static_cast<CheckBox*>(but)->on);
}

void Program::eventSetTheme(Button* but) {
	World::drawSys()->setTheme(static_cast<SwitchBox*>(but)->getText());
	World::scene()->resetLayouts();
}

void Program::eventSetFont(Button* but) {
	LineEdit* le = static_cast<LineEdit*>(but);
	World::drawSys()->setFont(le->getText());
	World::scene()->resetLayouts();
	if (World::winSys()->sets.getFont() != le->getText())
		World::scene()->setPopup(ProgState::createPopupMessage("Invalid font."));
}

void Program::eventSetRenderer(Button* but) {
	World::winSys()->setRenderer(static_cast<SwitchBox*>(but)->getText());
	World::scene()->resetLayouts();
}

void Program::eventSetScrollSpeed(Button* but) {
	World::winSys()->sets.setScrollSpeed(static_cast<LineEdit*>(but)->getText());
}

void Program::eventSetDeadzoneSL(Button* but) {
	World::winSys()->sets.setDeadzone(static_cast<Slider*>(but)->getVal());
	static_cast<LineEdit*>(but->getParent()->getWidget(2))->setText(ntos(World::winSys()->sets.getDeadzone()));	// update line edit
}

void Program::eventSetDeadzoneLE(Button* but) {
	LineEdit* le = static_cast<LineEdit*>(but);
	World::winSys()->sets.setDeadzone(stoi(le->getText()));
	le->setText(ntos(World::winSys()->sets.getDeadzone()));	// set text again in case the volume was out of range
	static_cast<Slider*>(but->getParent()->getWidget(1))->setVal(World::winSys()->sets.getDeadzone());	// update slider
}

// OTHER

void Program::eventClosePopup(Button* but) {
	World::scene()->setPopup(nullptr);
}

void Program::eventExit(Button* but) {
	World::winSys()->close();
}

void Program::setState(ProgState* newState) {
	state->eventClosing();
	state.reset(newState);
	World::scene()->resetLayouts();
}
