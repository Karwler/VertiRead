#include "library.h"
#include "engine/filer.h"

Library::Library(string FONT, const map<string, string>& TEXS, const map<string, string>& SNDS, string LANG)
{
	if (!fs::exists(FONT))
		FONT = VideoSettings().FontPath();	// should give the default font
	fonts = new FontSet(FONT);

	for (const pair<string, string>& it : TEXS) {
		SDL_Surface* test = IMG_Load(it.second.c_str());	// add only valid textures
		if (test) {
			texes.insert(make_pair(it.first, Texture(it.second)));
			SDL_FreeSurface(test);
		}
	}

	for (const pair<string, string>& it : SNDS) {
		Mix_Chunk* test = Mix_LoadWAV(it.second.c_str());	// add only valid sound files
		if (test) {
			sounds.insert(it);
			Mix_FreeChunk(test);
		}
	}

	LoadLanguage(LANG);
}

Library::~Library() {
	for (const pair<string, Texture>& it : texes)
		SDL_DestroyTexture(it.second.tex);
}

FontSet* Library::Fonts() const {
	return fonts;
}

string Library::getTexPath(string name) const {
	return texes.at(name).File();
}

Texture* Library::getTex(string name) const {
	return const_cast<Texture*>(&texes.at(name));
}

string Library::getSound(string name) const {
	return sounds.at(name);
}

string Library::getLine(string name) const {
	return lines.count(name) == 0 ? name : lines.at(name);
}

void Library::LoadLanguage(string language) {
	lines = Filer::GetLines(language + ".ini");
}

vector<Texture*> Library::Pictures() {
	vector<Texture*> txs(pics.size());
	for (uint i=0; i!=pics.size(); i++)
		txs[i] = &pics[i];
	return txs;
}

void Library::LoadPics(const vector<string>& files) {
	for (const string& it : files) {
		Texture tx(it);
		if (!tx.Res().hasNull())
			pics.push_back(tx);
		else if (tx.tex)
			SDL_DestroyTexture(tx.tex);
	}
}

void Library::ClearPics() {
	for (Texture& it : pics)
		SDL_DestroyTexture(it.tex);
	pics.clear();
}
