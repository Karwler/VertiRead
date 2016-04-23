#include "scene.h"
#include "world.h"

Scene::Scene() :
	program(new Program),
	library(new Library(World::winSys()->Settings().FontPath(), Filer::GetTextures(), Filer::GetSounds())),
	objectHold(nullptr)
{}

Scene::~Scene() {
	Clear(objects);
}

void Scene::SwitchMenu(EMenu newMenu, void* dat) {
	// reset values
	Clear(objects);
	focObject = 0;
	objectHold = nullptr;
	popup.reset();

	// little conveniences
	vec2i res = World::winSys()->Resolution();
	vec2i posT(-1, -1);
	vec2i sizT(140, 40);

	switch (newMenu) {
	case EMenu::books: {
		// top buttons
		objects.push_back(new ButtonText(Object(vec2i(0,         0), posT, vec2i(res.x/3-10, 50), FIX_Y | FIX_H), &Program::Event_OpenPlaylistList, "Playlists"));
		objects.push_back(new ButtonText(Object(vec2i(res.x/3,   0), posT, vec2i(res.x/3-10, 50), FIX_Y | FIX_H), &Program::Event_OpenGeneralSettings, "Settings"));
		objects.push_back(new ButtonText(Object(vec2i(res.x/3*2, 0), posT, vec2i(res.x/3,    50), FIX_Y | FIX_H), &Program::Event_Back, "Exit"));

		// book list
		vector<ListItem*> tiles;
		vector<fs::path> names = Filer::ListDir(Filer::dirLib(), FILTER_DIR);
		for (fs::path& it : names)
			tiles.push_back(new ItemButton(it.filename().string(), it.string(), &Program::Event_OpenBrowser));
		objects.push_back(new TileBox(Object(vec2i(0, 60), posT, vec2i(res.x, res.y-60), FIX_POS | FIX_END), tiles, vec2i(400, 30)));
		focObject = objects.size() - 1;
		break; }
	case EMenu::browser: {
		// back button
		objects.push_back(new ButtonText(Object(vec2i(0, 0), posT, vec2i(90, 40), FIX_POS | FIX_SIZ), &Program::Event_Back, "Back"));

		// list
		vector<ListItem*> items;
		Browser* browser = static_cast<Browser*>(dat);
		for (fs::path& it : browser->ListDirs())
			items.push_back(new ItemButton(it.filename().string(), "", &Program::Event_OpenBrowser));
		for (fs::path& it : browser->ListFiles())
			items.push_back(new ItemButton(it.filename().string(), it.string(), &Program::Event_OpenReader));
		objects.push_back(new ListBox(Object(vec2i(100, 0), posT, vec2i(res.x-100, res.y), FIX_POS | FIX_END, EColor::background), items));
		focObject = objects.size() - 1;
		break; }
	case EMenu::reader: {
		// reader box
		fs::path file = cstr(dat);
		library->LoadPics(Filer::GetPicsFromDir(file.parent_path()));
		objects.push_back(new ReaderBox(library->Pictures(), file.string()));
		focObject = objects.size() - 1;
		break; }
	case EMenu::playlists: {
		// top buttons
		objects.push_back(new ButtonText(Object(vec2i(0,         0), posT, vec2i(res.x/3-10, 50), FIX_Y | FIX_H), &Program::Event_OpenBookList, "Library"));
		objects.push_back(new ButtonText(Object(vec2i(res.x/3,   0), posT, vec2i(res.x/3-10, 50), FIX_Y | FIX_H), &Program::Event_OpenGeneralSettings, "Settings"));
		objects.push_back(new ButtonText(Object(vec2i(res.x/3*2, 0), posT, vec2i(res.x/3,    50), FIX_Y | FIX_H), &Program::Event_Back, "Exit"));

		// option buttons
		sizT = vec2i(80, 30);
		objects.push_back(new ButtonText(Object(vec2i(0,   60), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_AddButtonClick, "New"));
		objects.push_back(new ButtonText(Object(vec2i(90,  60), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_EditButtonClick, "Edit"));
		objects.push_back(new ButtonText(Object(vec2i(180, 60), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_DeleteButtonClick, "Del"));

		// playlist list
		TileBox* box = new TileBox(Object(vec2i(0, 100), posT, vec2i(res.x, res.y-100), FIX_POS | FIX_END), vector<ListItem*>(), vec2i(400, 30));
		vector<ListItem*> tiles;
		vector<fs::path> names = Filer::ListDir(Filer::dirPlist(), FILTER_FILE);
		for (fs::path& it : names)
			tiles.push_back(new ListItem(it.filename().string(), box));
		box->Items(tiles);
		objects.push_back(box);
		focObject = objects.size() - 1;
		break; }
	case EMenu::plistEditor: {
		// option buttons
		sizT.x = 100;
		objects.push_back(new ButtonText(Object(vec2i(0, 50),  posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_AddButtonClick, "Add"));
		objects.push_back(new ButtonText(Object(vec2i(0, 100), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_EditButtonClick, "Edit"));
		objects.push_back(new ButtonText(Object(vec2i(0, 150), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_DeleteButtonClick, "Del"));
		objects.push_back(new ButtonText(Object(vec2i(0, 200), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_SaveButtonClick, "Save"));
		objects.push_back(new ButtonText(Object(vec2i(0, 250), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_Back, "Close"));

		// playlist list
		ListBox* box = new ListBox(Object(vec2i(110, 0), posT, vec2i(res.x-110, res.y), FIX_POS | FIX_END, EColor::background));
		vector<ListItem*> items;
		PlaylistEditor* editor = static_cast<PlaylistEditor*>(dat);
		if (editor->showSongs) {
			objects.push_back(new ButtonText(Object(vec2i(0, 0), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_SwitchButtonClick, "Books"));
			for (fs::path& it : editor->getPlaylist().songs)
				items.push_back(new ItemButton(it.filename().string(), "", &Program::Event_SelectionSet, box));
		}
		else {
			objects.push_back(new ButtonText(Object(vec2i(0, 0), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_SwitchButtonClick, "Songs"));
			for (string& it : editor->getPlaylist().books)
				items.push_back(new ItemButton(it, "", &Program::Event_SelectionSet, box));
		}
		box->Items(items);
		objects.push_back(box);
		break; }
	case EMenu::generalSets:
		objects.push_back(new ButtonText(Object(vec2i(0, 0),   posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_OpenVideoSettings, "Video"));
		objects.push_back(new ButtonText(Object(vec2i(0, 50),  posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_OpenAudioSettings, "Audio"));
		objects.push_back(new ButtonText(Object(vec2i(0, 100), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_OpenControlsSettings, "Controls"));
		objects.push_back(new ButtonText(Object(vec2i(0, 150), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_Back, "Back"));
		break;
	case EMenu::videoSets:
		objects.push_back(new ButtonText(Object(vec2i(0, 0),   posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_OpenGeneralSettings, "General"));
		objects.push_back(new ButtonText(Object(vec2i(0, 50),  posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_OpenAudioSettings, "Audio"));
		objects.push_back(new ButtonText(Object(vec2i(0, 100), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_OpenControlsSettings, "Controls"));
		objects.push_back(new ButtonText(Object(vec2i(0, 150), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_Back, "Back"));
		break;
	case EMenu::audioSets:
		objects.push_back(new ButtonText(Object(vec2i(0, 0),   posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_OpenGeneralSettings, "General"));
		objects.push_back(new ButtonText(Object(vec2i(0, 50),  posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_OpenVideoSettings, "Video"));
		objects.push_back(new ButtonText(Object(vec2i(0, 100), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_OpenControlsSettings, "Controls"));
		objects.push_back(new ButtonText(Object(vec2i(0, 150), posT, sizT, FIX_POS | FIX_SIZ), &Program::Event_Back, "Back"));
		break;
	case EMenu::controlsSets:
		objects.push_back(new ButtonText(Object(vec2i(0, 0),   posT, sizT, FIX_ALL), &Program::Event_OpenGeneralSettings, "General"));
		objects.push_back(new ButtonText(Object(vec2i(0, 50),  posT, sizT, FIX_ALL), &Program::Event_OpenVideoSettings, "Video"));
		objects.push_back(new ButtonText(Object(vec2i(0, 100), posT, sizT, FIX_ALL), &Program::Event_OpenAudioSettings, "Audio"));
		objects.push_back(new ButtonText(Object(vec2i(0, 150), posT, sizT, FIX_ALL), &Program::Event_Back, "Back"));
	}
	World::engine->SetRedrawNeeded();
}

void Scene::ResizeMenu() {
	// for some reason the next three lines crash which is why they've been temporarily moved to the WindowSystem::DrawObjects function
	for (Object* obj : objects) {
		ScrollArea* box = dynamic_cast<ScrollArea*>(obj);
		if (box)
			box->SetValues();
	}
	World::engine->SetRedrawNeeded();
}

void Scene::Tick() {
	// handle keyhold
	if (InputSys::isPressed(SDL_SCANCODE_UP))
		program->Event_Up();
	else if (InputSys::isPressed(SDL_SCANCODE_DOWN))
		program->Event_Down();
	else if (InputSys::isPressed(SDL_SCANCODE_LEFT))
		program->Event_Left();
	else if (InputSys::isPressed(SDL_SCANCODE_RIGHT))
		program->Event_Right();

	// handle mousemove
	if (objectHold) {
		ReaderBox* box = dynamic_cast<ReaderBox*>(objectHold);
		if (box && !box->sliderFocused) {
			if (InputSys::isPressed(SDL_SCANCODE_LSHIFT))
				box->ScrollListX(-World::inputSys()->mouseMove().x);
			objectHold->ScrollList(-World::inputSys()->mouseMove().y);
		}
		else
			objectHold->DragSlider(InputSys::mousePos().y);
	}

	// object ticks
	for (Object* it : objects) {
		if (dynamic_cast<ReaderBox*>(it))
			static_cast<ReaderBox*>(it)->Tick();
		else if (dynamic_cast<Popup*>(it))
			static_cast<Popup*>(it)->Tick();
	}
}

void Scene::OnMouseDown() {
	// check if there's a popup window
	if (popup) {
		if (!inRect(popup->getRect(), InputSys::mousePos()))
			return;

		if (dynamic_cast<PopupChoice*>(popup.get())) {
			if (CheckPopupChoiceClick(static_cast<PopupChoice*>(popup.get())))
				return;
		}
		else if (dynamic_cast<PopupMessage*>(popup.get())) {
			if (CheckPopupSimpleClick(static_cast<PopupMessage*>(popup.get())))
				return;
		}
		return;
	}

	// check other objects
	for (Object* obj : objects) {
		if (!inRect(obj->getRect(), InputSys::mousePos()))	// skip if mouse isn't over object
			continue;

		if (dynamic_cast<Button*>(obj)) {
			dynamic_cast<Button*>(obj)->OnClick();
			break;
		}
		else if (dynamic_cast<ScrollArea*>(obj)) {
			if (CheckSliderClick(static_cast<ScrollArea*>(obj)))	// first check if slider is clicked
				break;
			else if (dynamic_cast<ListBox*>(obj)) {
				if (CheckListBoxClick(static_cast<ListBox*>(obj)))
					break;
			}
			else if (dynamic_cast<TileBox*>(obj)) {
				if (CheckTileBoxClick(static_cast<TileBox*>(obj)))
					break;
			}
			else if (dynamic_cast<ReaderBox*>(obj)) {
				if (CheckReaderBoxClick(static_cast<ReaderBox*>(obj)))
					break;
			}
		}

	}
}

bool Scene::CheckSliderClick(ScrollArea* obj) {
	vec2i mPos = InputSys::mousePos();
	if (inRect(obj->Bar(), mPos)) {
		objectHold = obj;
		if (mPos.y < obj->SliderY() || mPos.y > obj->SliderY() + obj->SliderH())	// if mouse outside of slider but inside bar
			obj->DragSlider(mPos.y - obj->SliderH() /2);
		obj->diffSliderMouseY = mPos.y - obj->SliderY();	// get difference between mouse y and slider y

		ReaderBox* box = dynamic_cast<ReaderBox*>(obj);
		if (box)
			box->sliderFocused = true;
		return true;
	}
	return false;
}

bool Scene::CheckListBoxClick(ListBox* obj) {
	vec2i interval = obj->VisibleItems();
	vector<ListItem*> items = obj->Items();
	for (int i=interval.x; i<=interval.y; i++) {
		SDL_Rect crop;
		SDL_Rect rect = obj->ItemRect(i, &crop);
		if (inRect(rect, InputSys::mousePos())) {
			items[i]->OnClick();
			return true;
		}
	}
	return false;
}

bool Scene::CheckTileBoxClick(TileBox* obj) {
	vec2i interval = obj->VisibleItems();
	const vector<ListItem*>& items = obj->Items();
	for (int i=interval.x; i<=interval.y; i++)
		if (inRect(obj->ItemRect(i), InputSys::mousePos())) {
			items[i]->OnClick();
			return true;
		}
	return false;
}

bool Scene::CheckReaderBoxClick(ReaderBox* obj) {
	vec2i mPos = InputSys::mousePos();
	if (obj->showList())	// check list buttons
		for (ButtonImage& but : obj->ListButtons())
			if (inRect(but.getRect(), mPos)) {
				but.OnClick();
				return true;
			}
	if (obj->showPlayer())	// check player buttons
		for (ButtonImage& but : obj->PlayerButtons())
			if (inRect(but.getRect(), mPos)) {
				but.OnClick();
				return true;
			}
	if (inRect({obj->Pos().x, obj->Pos().y, obj->Size().x-obj->barW, obj->Size().y}, mPos)) {	// init list mouse drag
		objectHold = obj;
		return true;
	}
	return false;
}

bool Scene::CheckPopupSimpleClick(PopupMessage* obj) {
	if (inRect(obj->CancelButton(), InputSys::mousePos())) {
		SetPopup(nullptr);
		return true;
	}
	return false;
}

bool Scene::CheckPopupChoiceClick(PopupChoice* obj) {
	if (inRect(obj->OkButton(), InputSys::mousePos()))
		program->Event_PopupOk(obj);
	else if (inRect(obj->CancelButton(), InputSys::mousePos()))
		SetPopup(nullptr);
	else
		return false;
	return true;
}

void Scene::OnMouseUp() {
	if (objectHold) {
		// reset values
		ReaderBox* box = dynamic_cast<ReaderBox*>(objectHold);
		if (box)
			box->sliderFocused = false;

		objectHold->diffSliderMouseY = 0;
		objectHold = nullptr;
	}
}

void Scene::OnMouseWheel(int ymov) {
	ScrollArea* box = dynamic_cast<ScrollArea*>(FocusedObject());
	if (box)
		box->ScrollList(ymov*-20);
}

Program* Scene::getProgram() const {
	return program;
}

Library* Scene::getLibrary() const {
	return library;
}

vector<Object*> Scene::Objects() const {
	vector<Object*> objs = objects;	// return objects
	objs.push_back(popup);			// append popup
	return objs;
}

Object* Scene::FocusedObject() const {
	return (popup) ? popup : (objects[focObject]) ? objects[focObject] : nullptr;
}

ListItem* Scene::SelectedButton() const {
	// check focused object first
	ScrollArea* sabox = dynamic_cast<ScrollArea*>(FocusedObject());
	if (sabox)
		if (sabox->selectedItem && sabox->selectedItem->selectable())
			return sabox->selectedItem;

	// if failed, look through all objects
	for (Object* obj : objects) {
		ScrollArea* box = dynamic_cast<ScrollArea*>(obj);
		if (box)
			if (box->selectedItem && box->selectedItem->selectable())
				return box->selectedItem;
	}
	return nullptr;	// nothing found
}

bool Scene::ShowingPopup() const {
	return popup != nullptr;
}

void Scene::SetPopup(Popup* box) {
	popup = box;

	PopupText* poptext = dynamic_cast<PopupText*>(box);
	if (poptext)
		World::inputSys()->SetCapture(poptext->Line());

	World::engine->SetRedrawNeeded();
}
