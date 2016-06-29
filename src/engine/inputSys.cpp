#include "world.h"

InputSys::InputSys(const ControlsSettings& SETS) :
	sets(SETS)
{
	SDL_GetMouseState(&lastMousePos.x, &lastMousePos.y);
	ResetCapture();
}

void InputSys::Tick() {
	lastMousePos = mousePos();
}

void InputSys::KeypressEvent(const SDL_KeyboardEvent& key) {
	// different behaviour when capturing or not
	if (curCap == ECapture::CP)
		capCP->OnKeypress(key.keysym.scancode);
	else if (curCap == ECapture::LE)
		capLE->OnKeypress(key.keysym.scancode);
	else if (!key.repeat)	// handle only once pressed keys
		CheckShortcuts(key);
}

void InputSys::MouseButtonDownEvent(const SDL_MouseButtonEvent& button) {
	if (curCap != ECapture::NONE && !World::scene()->getPopup()) {	// mouse button cancels keyboard capture (except if popup is shown)
		// confirm entered text if necessary
		if (curCap == ECapture::CP) {
			if (LineEdit* box = dynamic_cast<LineEdit*>(capCP))
				box->Confirm();
		}
		else
			capLE->Confirm();

		ResetCapture();
	}

	if (button.clicks == 1) {
		if (button.button == SDL_BUTTON_LEFT)		// single left click
			World::scene()->OnMouseDown(EClick::left);
		else if (button.button == SDL_BUTTON_RIGHT)	// single right click
			World::scene()->OnMouseDown(EClick::right);
	}
	else if (button.button == SDL_BUTTON_LEFT)		// double left click
		World::scene()->OnMouseDown(EClick::left_double);
}

void InputSys::MouseButtonUpEvent(const SDL_MouseButtonEvent& button) {
	if (button.clicks == 1 && button.button == SDL_BUTTON_LEFT)
		World::scene()->OnMouseUp();	// left up
}

void InputSys::MouseWheelEvent(const SDL_MouseWheelEvent& wheel) {
	World::scene()->OnMouseWheel(wheel.y * int(sets.scrollSpeed.y) /2);
}

void InputSys::TextEvent(const SDL_TextInputEvent& text) {
	if (curCap == ECapture::CP)										// text input should only run if line edit is being captured, 
		static_cast<LineEdit*>(capCP)->Editor()->Add(text.text);	// therefore a cast check isn't necessary
	else
		capLE->Editor()->Add(text.text);
}

void InputSys::CheckShortcuts(const SDL_KeyboardEvent& key) {
	// find first shortcut with this key assigned to it
	for (const pair<string, Shortcut>& sc : sets.shortcuts)
		if (sc.second.key == key.keysym.scancode) {
			(World::program()->*sc.second.call)();
			break;
		}
}

bool InputSys::isPressed(const string& key) const {
	return SDL_GetKeyboardState(nullptr)[sets.holders.at(key)];
}

bool InputSys::isPressed(SDL_Scancode key) {
	return SDL_GetKeyboardState(nullptr)[key];
}

bool InputSys::isPressed(uint8 button) {
	return SDL_GetMouseState(nullptr, nullptr) & button;
}

vec2i InputSys::mousePos() {
	vec2i pos;
	SDL_GetMouseState(&pos.x, &pos.y);
	return pos;
}

vec2i InputSys::mouseMove() const {
	return mousePos() - lastMousePos;
}

ControlsSettings InputSys::Settings() const {
	return sets;
}

void InputSys::ScrollSpeed(const vec2f& sspeed) {
	sets.scrollSpeed = sspeed;
}

SDL_Scancode* InputSys::GetKeyPtr(const string& name, bool shortcut) {
	return shortcut ? &sets.shortcuts[name].key : &sets.holders[name];
}

Capturer* InputSys::CapturedCP() const {
	return capCP;
}

Capturer* InputSys::CapturedLE() const {
	return capLE;
}

ECapture InputSys::CurCaptured() const {
	return curCap;
}

void InputSys::ResetCapture() {
	// do some cleanup first
	if (curCap == ECapture::CP) {
		if (LineEdit* edit = dynamic_cast<LineEdit*>(capCP))
			edit->ResetTextPos();
		capCP = nullptr;
	}
	else if (curCap == ECapture::LE) {
		capLE->ResetTextPos();
		capLE = nullptr;
	}
	curCap = ECapture::NONE;
	SDL_StopTextInput();
}

void InputSys::SetCaptureCP(Capturer* cbox) {
	ResetCapture();

	capCP = cbox;
	curCap = ECapture::CP;
	if (dynamic_cast<LineEdit*>(capCP))
		SDL_StartTextInput();

	World::engine->SetRedrawNeeded();
}

void InputSys::SetCaptureLE(LineEditor* cbox) {
	ResetCapture();

	capLE = cbox;
	curCap = ECapture::LE;
	SDL_StartTextInput();
	
	World::engine->SetRedrawNeeded();
}
