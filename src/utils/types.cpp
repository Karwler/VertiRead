#include "engine/world.h"

// TEXTURE

Texture::Texture(const string& FILE) :
	surface(nullptr)
{
	LoadSurface(FILE);
}

Texture::Texture(const string& FILE, SDL_Surface* SURF) :
	surface(SURF),
	file(FILE)
{}

string Texture::File() const {
	return file;
}

vec2i Texture::Res() const {
	return surface ? vec2i(surface->w, surface->h) : 0;
}

void Texture::LoadSurface(const string& path) {
	file = path;
	if (surface)
		SDL_FreeSurface(surface);
	
	surface = IMG_Load(file.c_str());
	if (!surface)
		cerr << "couldn't load surface " << file << endl;
}

// IMAGE

Image::Image(const vec2i& POS, Texture* TEX, const vec2i& SIZ) :
	pos(POS),
	texture(TEX)
{
	size = SIZ.hasNull() ? texture->Res() : SIZ;
}

Image::Image(const vec2i& POS, const string& TEX, const vec2i& SIZ) :
	pos(POS)
{
	texture = World::library()->getTex(TEX);
	size = SIZ.hasNull() ? texture->Res() : SIZ;
}

SDL_Rect Image::getRect() const {
	return { pos.x, pos.y, size.x, size.y };
}

// FONT

bool FontSet::Initialize(const string& FILE) {
	TTF_Font* tmp = TTF_OpenFont(FILE.c_str(), 72);
	if (!tmp) {
		cerr << "couldn't open font " << file << endl << TTF_GetError() << endl;
		file.clear();
		return false;
	}
	TTF_CloseFont(tmp);
	file = FILE;
	return true;
}

void FontSet::Clear() {
	for (const pair<int, TTF_Font*>& it : fonts)
		TTF_CloseFont(it.second);
	fonts.clear();
}

bool FontSet::CanRun() const {
	return !file.empty();
}

TTF_Font* FontSet::Get(int size) {
	return (CanRun() && fonts.count(size) == 0) ? AddSize(size) : fonts.at(size);
}

TTF_Font* FontSet::AddSize(int size) {
	TTF_Font* font = TTF_OpenFont(file.c_str(), size);
	if (font)
		fonts.insert(make_pair(size, font));
	else
		cerr << "couldn't load font " << file << endl << TTF_GetError() << endl;
	return font;
}

vec2i FontSet::TextSize(const string& text, int size) {
	vec2i siz;
	TTF_Font* font = Get(size);
	if (font)
		TTF_SizeUTF8(font, text.c_str(), &siz.x, &siz.y);
	return siz;
}

// TEXT

Text::Text(const string& TXT, const vec2i& POS, int H, EColor CLR, int HSCAL) :
	pos(POS),
	color(CLR),
	text(TXT)
{
	height = (HSCAL == 0) ? H : H - H/HSCAL;
}

void Text::SetPosToRect(const SDL_Rect& rect, ETextAlign align, int offset) {
	int len = size().x;

	switch (align) {
	case ETextAlign::left:
		pos.x = rect.x+offset;
		break;
	case ETextAlign::center:
		pos.x = rect.x + (rect.w-len)/2;
		break;
	case ETextAlign::right:
		pos.x = rect.x+rect.w-len-offset;
	}
	pos.y = rect.y;
}

vec2i Text::size() const {
	return World::library()->Fonts()->TextSize(text, height);
}

// TEXT EDIT

TextEdit::TextEdit(const string& TXT, ETextType TYPE, size_t CPOS) :
	type(TYPE),
	text(TXT)
{
	SetCursor(CPOS);
	CheckText();
}

size_t TextEdit::CursorPos() const {
	return cpos;
}

void TextEdit::SetCursor(size_t pos) {
	cpos = (pos > text.length()) ? text.length() : pos;
}

void TextEdit::MoveCursor(bool right, bool loop) {
	if (loop) {
		if (right)
			cpos = (cpos == text.length()) ? 0 : cpos+1;
		else
			cpos = (cpos == 0) ? text.length() : cpos-1;
	}
	else {
		if (right && cpos != text.length())
			cpos++;
		else if (!right && cpos != 0)
			cpos--;
	}
}

string TextEdit::Text() const {
	return text;
}

void TextEdit::Text(const string& str, bool resetCpos) {
	text = str;
	
	if (resetCpos)
		cpos = 0;
	else
		CheckCaret();
}

void TextEdit::Add(const string& str) {
	text.insert(cpos, str);
	cpos += str.length();
	CheckText();
}

void TextEdit::Delete(bool current) {
	if (current) {
		if (cpos != text.length())
			text.erase(cpos, 1);
	}
	else if (cpos != 0) {
		cpos--;
		text.erase(cpos, 1);
	}
}

void TextEdit::CheckCaret() {
	if (cpos > text.length())
		cpos = text.length();
}

void TextEdit::CheckText() {
	if (type == ETextType::text)
		return;

	bool foundDot = false;
	for (size_t i=0; i!=text.length(); i++) {
		if (type == ETextType::integer) {
			if (text[i] < '0' || text[i] > '9') {
				text.erase(i, 1);
				i--;
			}
		}
		else if (type == ETextType::floating) {
			if (text[i] == '.') {
				if (foundDot) {
					text.erase(i, 1);
					i--;
				}
				else
					foundDot = true;
			}
			else if (text[i] < '0' || text[i] > '9') {
				text.erase(i, 1);
				i--;
			}
		}
	}
	CheckCaret();
}

// CONTROLLER

Controller::Controller() :
	joystick(nullptr),
	gamepad(nullptr)
{}

bool Controller::Open(int id) {
	if (SDL_IsGameController(id))
		gamepad = SDL_GameControllerOpen(id);

	joystick = (gamepad) ? SDL_GameControllerGetJoystick(gamepad) : SDL_JoystickOpen(id);
	return joystick != nullptr;
}

void Controller::Close() {
	if (gamepad) {
		SDL_GameControllerClose(gamepad);
		gamepad = nullptr;
		joystick = nullptr;
	}
	else if (joystick) {
		SDL_JoystickClose(joystick);
		joystick = nullptr;
	}
}

// SHORTCUT

Shortcut::Shortcut()
{
	ClearAsgKey();
	ClearAsgCtr();
}

Shortcut::Shortcut(SDL_Scancode KEY)
{
	Key(KEY);
	ClearAsgCtr();
}

Shortcut::Shortcut(uint8 CTR, EAssgnment ASG)
{
	ClearAsgKey();
	switch (ASG) {
	case ASG_JBUTTON:
		JButton(CTR);
		break;
	case ASG_JAXIS_P:
		JAxis(CTR, true);
		break;
	case ASG_JAXIS_N:
		JAxis(CTR, false);
		break;
	case ASG_GBUTTON:
		GButton(CTR);
		break;
	case ASG_GAXIS_P:
		GAxis(CTR, true);
		break;
	case ASG_GAXIS_N:
		GAxis(CTR, false);
		break;
	default:
		ClearAsgCtr();
	}
}

Shortcut::Shortcut(uint8 HAT, uint8 VAL)
{
	ClearAsgKey();
	JHat(HAT, VAL);
}

Shortcut::Shortcut(SDL_Scancode KEY, uint8 CTR, EAssgnment ASG)
{
	Key(KEY);
	switch (ASG) {
	case ASG_JBUTTON:
		JButton(CTR);
		break;
	case ASG_JAXIS_P:
		JAxis(CTR, true);
		break;
	case ASG_JAXIS_N:
		JAxis(CTR, false);
		break;
	case ASG_GBUTTON:
		GButton(CTR);
		break;
	case ASG_GAXIS_P:
		GAxis(CTR, true);
		break;
	case ASG_GAXIS_N:
		GAxis(CTR, false);
		break;
	default:
		ClearAsgCtr();
	}
}

Shortcut::Shortcut(SDL_Scancode KEY, uint8 HAT, uint8 VAL)
{
	Key(KEY);
	JHat(HAT, VAL);
}

Shortcut::~Shortcut() {}

SDL_Scancode Shortcut::Key() const {
	return key;
}

bool Shortcut::KeyAssigned() const {
	return asg & ASG_KEY;
}

void Shortcut::Key(SDL_Scancode KEY) {
	key = KEY;
	asg |= ASG_KEY;
}

void Shortcut::ClearAsgKey() {
	asg ^= asg & ASG_KEY;
}

uint8 Shortcut::CtrID() const {
	return ctrID;
}

bool Shortcut::JButtonAssigned() const {
	return asg & ASG_JBUTTON;
}

void Shortcut::JButton(uint8 BUT) {
	ctrID = BUT;

	ClearAsgCtr();
	asg |= ASG_JBUTTON;
}

bool Shortcut::JAxisAssigned() const {
	return (asg & ASG_JAXIS_P) || (asg & ASG_JAXIS_N);
}

bool Shortcut::JPosAxisAssigned() const {
	return asg & ASG_JAXIS_P;
}

bool Shortcut::JNegAxisAssigned() const {
	return asg & ASG_JAXIS_N;
}

void Shortcut::JAxis(uint8 AXIS, bool positive) {
	ctrID = AXIS;

	ClearAsgCtr();
	asg |= (positive) ? ASG_JAXIS_P : ASG_JAXIS_N;
}

uint8 Shortcut::JHatVal() const {
	return jHatVal;
}

bool Shortcut::JHatAssigned() const {
	return asg & ASG_JHAT;
}

void Shortcut::JHat(uint8 HAT, uint8 VAL) {
	ctrID = HAT;
	jHatVal = VAL;

	ClearAsgCtr();
	asg |= ASG_JHAT;
}

bool Shortcut::GButtonAssigned() const {
	return asg & ASG_GBUTTON;
}

void Shortcut::GButton(uint8 BUT) {
	ctrID = BUT;

	ClearAsgCtr();
	asg = ASG_GBUTTON;
}

bool Shortcut::GAxisAssigned() const {
	return (asg & ASG_GAXIS_P) || (asg & ASG_GAXIS_N);
}

bool Shortcut::GPosAxisAssigned() const {
	return asg & ASG_GAXIS_P;
}

bool Shortcut::GNegAxisAssigned() const {
	return asg & ASG_GAXIS_N;
}

void Shortcut::GAxis(uint8 AXIS, bool positive) {
	ctrID = AXIS;

	ClearAsgCtr();
	asg |= (positive) ? ASG_GAXIS_P : ASG_GAXIS_N;
}

void Shortcut::ClearAsgCtr() {
	asg &= ASG_KEY;
}

Shortcut::EAssgnment operator~(Shortcut::EAssgnment a) {
	return static_cast<Shortcut::EAssgnment>(~static_cast<uint8>(a));
}
Shortcut::EAssgnment operator&(Shortcut::EAssgnment a, Shortcut::EAssgnment b) {
	return static_cast<Shortcut::EAssgnment>(static_cast<uint8>(a) & static_cast<uint8>(b));
}
Shortcut::EAssgnment operator&=(Shortcut::EAssgnment& a, Shortcut::EAssgnment b) {
	return a = static_cast<Shortcut::EAssgnment>(static_cast<uint8>(a) & static_cast<uint8>(b));
}
Shortcut::EAssgnment operator^(Shortcut::EAssgnment a, Shortcut::EAssgnment b) {
	return static_cast<Shortcut::EAssgnment>(static_cast<uint8>(a) ^ static_cast<uint8>(b));
}
Shortcut::EAssgnment operator^=(Shortcut::EAssgnment& a, Shortcut::EAssgnment b) {
	return a = static_cast<Shortcut::EAssgnment>(static_cast<uint8>(a) ^ static_cast<uint8>(b));
}
Shortcut::EAssgnment operator|(Shortcut::EAssgnment a, Shortcut::EAssgnment b) {
	return static_cast<Shortcut::EAssgnment>(static_cast<uint8>(a) | static_cast<uint8>(b));
}
Shortcut::EAssgnment operator|=(Shortcut::EAssgnment& a, Shortcut::EAssgnment b) {
	return a = static_cast<Shortcut::EAssgnment>(static_cast<uint8>(a) | static_cast<uint8>(b));
}

// SHORTCUT KEY

ShortcutKey::ShortcutKey(void (Program::*CALL)()) :
	Shortcut(),
	call(CALL)
{}

ShortcutKey::ShortcutKey(SDL_Scancode KEY, void (Program::*CALL)()) :
	Shortcut(KEY),
	call(CALL)
{}

ShortcutKey::ShortcutKey(uint8 CTR, EAssgnment ASG, void (Program::*CALL)()) :
	Shortcut(CTR, ASG),
	call(CALL)
{}

ShortcutKey::ShortcutKey(uint8 HAT, uint8 VAL, void (Program::*CALL)()) :
	Shortcut(HAT, VAL),
	call(CALL)
{}

ShortcutKey::ShortcutKey(SDL_Scancode KEY, uint8 CTR, EAssgnment ASG, void (Program::*CALL)()) :
	Shortcut(KEY, CTR, ASG),
	call(CALL)
{}

ShortcutKey::ShortcutKey(SDL_Scancode KEY, uint8 HAT, uint8 VAL, void (Program::*CALL)()) :
	Shortcut(KEY, HAT, VAL),
	call(CALL)
{}

ShortcutKey::~ShortcutKey() {}

// SHORTCUT AXIS

ShortcutAxis::ShortcutAxis(void (Program::*CALL)(float)) :
	Shortcut(),
	call(CALL)
{}

ShortcutAxis::ShortcutAxis(SDL_Scancode KEY, void (Program::*CALL)(float)) :
	Shortcut(KEY),
	call(CALL)
{}

ShortcutAxis::ShortcutAxis(uint8 CTR, EAssgnment ASG, void (Program::*CALL)(float)) :
	Shortcut(CTR, ASG),
	call(CALL)
{}

ShortcutAxis::ShortcutAxis(uint8 HAT, uint8 VAL, void (Program::*CALL)(float)) :
	Shortcut(HAT, VAL),
	call(CALL)
{}

ShortcutAxis::ShortcutAxis(SDL_Scancode KEY, uint8 CTR, EAssgnment ASG, void (Program::*CALL)(float)) :
	Shortcut(KEY, CTR, ASG),
	call(CALL)
{}

ShortcutAxis::ShortcutAxis(SDL_Scancode KEY, uint8 HAT, uint8 VAL, void (Program::*CALL)(float)) :
	Shortcut(KEY, HAT, VAL),
	call(CALL)
{}

ShortcutAxis::~ShortcutAxis() {}

// PLAYLIST

Playlist::Playlist(const string& NAME, const vector<string>& SGS, const vector<string>& BKS) :
	name(NAME),
	songs(SGS),
	books(BKS)
{}

string Playlist::songPath(size_t id) const {
	return isAbsolute(songs[id]) ? songs[id] : parentPath(World::scene()->Settings().PlaylistPath() + name) + songs[id];
}

Directory::Directory(const string& NAME, const vector<string>& DIRS, const vector<string>& FILS) :
	name(NAME),
	dirs(DIRS),
	files(FILS)
{}

// EXCEPTION

Exception::Exception(const string& MSG, int RV) :
	message(MSG),
	retval(RV)
{}

void Exception::Display() {
	cerr << "ERROR: " << message << " (code " << retval << ")" << endl;
}
