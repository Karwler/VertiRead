#include "engine/world.h"

// CAPTURER

Capturer::Capturer(const Object& BASE) :
	Object(BASE)
{}
Capturer::~Capturer() {}

void Capturer::OnClick() {
	World::inputSys()->SetCapture(this);
}

// LINE EDIT

LineEdit::LineEdit(const Object& BASE, string TXT) :
	Capturer(BASE)
{}
LineEdit::~LineEdit() {}

void LineEdit::OnClick() {
	Capturer::OnClick();
	// set cursor pos
}

void LineEdit::OnKeypress(SDL_Scancode key) {
	switch (key) {
	case SDL_SCANCODE_LEFT:
		editor.MoveCursor(-1);
		break;
	case SDL_SCANCODE_RIGHT:
		editor.MoveCursor(1);
		break;
	case SDL_SCANCODE_BACKSPACE:
		editor.Delete(false);
		break;
	case SDL_SCANCODE_DELETE:
		editor.Delete(true);
		break;
	case SDL_SCANCODE_RETURN:
		World::program()->Event_TextEditConfirmed(&editor);
	}
}

void LineEdit::AddText(cstr text) {
	editor.Add(text);
}

Text LineEdit::getText(vec2i* sideCrop) const {
	return Text(editor.getText(), Pos(), Size().y, 8);
}

TextEdit* LineEdit::Editor() const {
	return const_cast<TextEdit*>(&editor);
}

// KEY GETTER

KeyGetter::KeyGetter(const Object& BASE, SDL_Scancode KEY) :
	Capturer(BASE),
	key(KEY)
{}
KeyGetter::~KeyGetter() {}

void KeyGetter::OnClick() {
	Capturer::OnClick();
	key = SDL_SCANCODE_ESCAPE;
}

void KeyGetter::OnKeypress(SDL_Scancode KEY) {
	key = KEY;
	World::inputSys()->SetCapture(nullptr);
}

Text KeyGetter::getText() const {
	return Text(SDL_GetScancodeName(key), Pos(), Size().y, 8);
}
