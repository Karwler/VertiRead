#include "world.h"

uint8 Filer::CheckDirectories(const GeneralSettings& sets) {
	uint8 retval = 0;
	if (!fs::exists(dirSets()))
		fs::create_directories(dirSets());
	if (!fs::exists(sets.LibraryParh()))
		fs::create_directories(sets.LibraryParh());
	if (!fs::exists(sets.PlaylistParh()))
		fs::create_directories(sets.PlaylistParh());
	if (!fs::exists(dirLangs())) {
		cerr << "couldn't find translation directory" << endl;
		retval = 1;
	}
	if (!fs::exists(dirSnds())) {
		cerr << "couldn't find sound directory" << endl;
		retval = 2;
	}
	if (!fs::exists(dirTexs())) {
		cerr << "couldn't find texture directory" << endl;
		retval = 3;
	}
	return retval;
}

bool Filer::ReadTextFile(const string& file, vector<string>& lines, bool printMessage) {
	std::ifstream ifs(file.c_str());
	if (!ifs.good()) {
		if (printMessage)
			cerr << "couldn't read file " << file << endl;
		return false;
	}
	lines.clear();
	for (string line; readLine(ifs, line);)
		lines.push_back(line);
	return true;
}

bool Filer::WriteTextFile(const string& file, const vector<string>& lines) {
	std::ofstream ofs(file.c_str());
	if (!ofs.good()) {
		cerr << "couldn't write file " << file << endl;
		return false;
	}
	for (const string& line : lines)
		ofs << line << endl;
	return true;
}

vector<fs::path> Filer::ListDir(const fs::path& dir, EDirFilter filter, const vector<string>& extFilter) {
	vector<fs::path> names;
	if (!fs::is_directory(dir))
		return names;

	for (fs::directory_iterator it(dir); it!=fs::directory_iterator(); it++) {
		if (filter == FILTER_ALL)
			names.push_back(it->path());
		else if ((filter & FILTER_FILE) && fs::is_regular_file(it->path())) {
			if (extFilter.empty())
				names.push_back(it->path());
			else for (const string& ext : extFilter)
				if (it->path().extension() == ext) {
					names.push_back(it->path());
					break;
				}
		}
		else if ((filter & FILTER_DIR) && fs::is_directory(it->path()))
			names.push_back(it->path());
		else if ((filter & FILTER_LINK) && fs::is_symlink(it->path()))
			names.push_back(it->path());
	}
	sort(names.begin(), names.end());
	return names;
}

vector<fs::path> Filer::ListDirRecursively(const fs::path& dir, EDirFilter filter, const vector<string>& extFilter) {
	vector<fs::path> names;
	for (fs::recursive_directory_iterator it(dir); it!=fs::recursive_directory_iterator(); it++) {
		if (filter == FILTER_ALL)
			names.push_back(it->path());
		else if ((filter & FILTER_FILE) && fs::is_regular_file(it->path())) {
			if (extFilter.empty())
				names.push_back(it->path());
			else for (const string& ext : extFilter)
				if (it->path().extension() == ext) {
					names.push_back(it->path());
					break;
				}
		}
		else if ((filter & FILTER_DIR) && fs::is_directory(it->path()))
			names.push_back(it->path());
		else if ((filter & FILTER_LINK) && fs::is_symlink(it->path()))
			names.push_back(it->path());
	}
	return names;
}

vector<string> Filer::GetAvailibleLanguages() {
	vector<string> files = { "english" };
	if (!fs::exists(dirLangs()))
		return files;

	for (fs::directory_iterator it(dirLangs()); it != fs::directory_iterator(); it++)
		if (fs::is_regular_file(it->path()) && it->path().extension() == ".ini")
			files.push_back(delExt(it->path().filename()).string());

	sort(files.begin(), files.end());
	return files;
}

map<string, string> Filer::GetLines(const string& language) {
	vector<string> lines;
	if (!ReadTextFile(dirLangs() + language + ".ini", lines, false))
		return map<string, string>();

	map<string, string> translation;
	for (string& line : lines) {
		string arg, val;
		if (splitIniLine(line, &arg, &val))
			translation.insert(make_pair(arg, val));
	}
	return translation;
}

map<string, Mix_Chunk*> Filer::GetSounds() {
	map<string, Mix_Chunk*> sounds;
	for (fs::directory_iterator it(dirSnds()); it != fs::directory_iterator(); it++)
		if (Mix_Chunk* cue = Mix_LoadWAV(it->path().string().c_str()))				// add only valid sound files
			sounds.insert(make_pair(delExt(it->path().filename()).string(), cue));
	return sounds;
}

map<string, Texture> Filer::GetTextures() {
	map<string, Texture> texes;
	for (fs::directory_iterator it(dirTexs()); it != fs::directory_iterator(); it++)
		if (SDL_Surface* surf = IMG_Load(it->path().string().c_str()))				// add only valid textures
			texes.insert(make_pair(delExt(it->path().filename()).string(), Texture(it->path().string(), surf)));
	return texes;
}

vector<string> Filer::GetPics(const fs::path& dir) {
	vector<string> pics;
	if (!fs::is_directory(dir))
		return pics;

	for (fs::directory_iterator it(dir); it != fs::directory_iterator(); it++)
		if (fs::is_regular_file(it->path()))
			pics.push_back(it->path().string());
	sort(pics.begin(), pics.end());
	return pics;
}

Playlist Filer::LoadPlaylist(const string& name) {
	vector<string> lines;
	if (!ReadTextFile(World::scene()->Settings().PlaylistParh() + name, lines))
		return Playlist();

	Playlist plist(name);
	for (string& line : lines) {
		string arg, val;
		if (!splitIniLine(line, &arg, &val))
			continue;

		if (arg == "book")
			plist.books.push_back(val);
		else if (arg == "song")
			plist.songs.push_back(line);
	}
	return plist;
}

void Filer::SavePlaylist(const Playlist& plist) {
	vector<string> lines;
	for (const string& name : plist.books)
		lines.push_back("book=" + name);
	for (const fs::path& file : plist.songs)
		lines.push_back(file.string());

	WriteTextFile(World::scene()->Settings().PlaylistParh() + plist.name, lines);
}

GeneralSettings Filer::LoadGeneralSettings() {
	vector<string> lines;
	if (!ReadTextFile(dirSets() + "general.ini", lines, false))
		return GeneralSettings();

	GeneralSettings sets;
	for (string& line : lines) {
		string arg, val;
		if (!splitIniLine(line, &arg, &val))
			continue;

		if (arg == "language")
			sets.Lang(val);
		else if (arg == "library")
			sets.DirLib(val);
		else if (arg == "playlists")
			sets.DirPlist(val);
	}
	return sets;
}

void Filer::SaveSettings(const GeneralSettings& sets) {
	vector<string> lines = {
		"language=" + sets.Lang(),
		"library=" + sets.DirLib(),
		"playlists=" + sets.DirPlist()
	};
	WriteTextFile(dirSets() + "general.ini", lines);
}

VideoSettings Filer::LoadVideoSettings() {
	vector<string> lines;
	if (!ReadTextFile(dirSets() + "video.ini", lines, false))
		return VideoSettings();

	VideoSettings sets;
	for (string& line : lines) {
		string arg, val, key;
		if (!splitIniLine(line, &arg, &val, &key))
			continue;

		if (arg == "font")
			sets.SetFont(val);
		else if (arg == "renderer")
			sets.renderer = val;
		else if (arg == "maximized")
			sets.maximized = stob(val);
		else if (arg == "fullscreen")
			sets.fullscreen = stob(val);
		else if (arg == "resolution") {
			vector<string> elems = getWords(val);
			if (elems.size() > 0) sets.resolution.x = stoi(elems[0]);
			if (elems.size() > 1) sets.resolution.y = stoi(elems[1]);
		}
		else if (arg == "color") {
			vector<string> elems = getWords(val);
			if (elems.size() > 0) sets.colors[EColor(stoi(key))].x = stoi(elems[0]);
			if (elems.size() > 1) sets.colors[EColor(stoi(key))].y = stoi(elems[1]);
			if (elems.size() > 2) sets.colors[EColor(stoi(key))].z = stoi(elems[2]);
			if (elems.size() > 3) sets.colors[EColor(stoi(key))].a = stoi(elems[3]);
		}
	}
	return sets;
}

void Filer::SaveSettings(const VideoSettings& sets) {
	vector<string> lines = {
		"font=" + sets.Font(),
		"renderer=" + sets.renderer,
		"maximized=" + btos(sets.maximized),
		"fullscreen=" + btos(sets.fullscreen),
		"resolution=" + to_string(sets.resolution.x) + ' ' + to_string(sets.resolution.y)
	};
	for (const pair<EColor, vec4b>& it : sets.colors)
		lines.push_back("color["+to_string(ushort(it.first))+"]=" + to_string(it.second.x) + ' ' + to_string(it.second.y) + ' ' + to_string(it.second.z) + ' ' + to_string(it.second.a));
	WriteTextFile(dirSets() + "video.ini", lines);
}

AudioSettings Filer::LoadAudioSettings() {
	vector<string> lines;
	if (!ReadTextFile(dirSets() + "audio.ini", lines, false))
		return AudioSettings();

	AudioSettings sets;
	for (string& line : lines) {
		string arg, val;
		if (!splitIniLine(line, &arg, &val))
			continue;

		if (arg == "music_vol")
			sets.musicVolume = stoi(val);
		else if (arg == "interface_vol")
			sets.soundVolume = stoi(val);
		else if (arg == "song_delay")
			sets.songDelay = stof(val);
	}
	return sets;
}

void Filer::SaveSettings(const AudioSettings& sets) {
	vector<string> lines = {
		"music_vol=" + to_string(sets.musicVolume),
		"interface_vol=" + to_string(sets.soundVolume),
		"song_delay=" + to_string(sets.songDelay)
	};
	WriteTextFile(dirSets() + "audio.ini", lines);
}

ControlsSettings Filer::LoadControlsSettings() {
	vector<string> lines;
	if (!ReadTextFile(dirSets() + "controls.ini", lines, false))
		return ControlsSettings();

	ControlsSettings sets;
	for (string& line : lines) {
		string arg, val, key;
		if (!splitIniLine(line, &arg, &val, &key))
			continue;

		if (arg == "scroll_speed") {
			vector<string> elems = getWords(val);
			if (elems.size() > 0) sets.scrollSpeed.x = stof(elems[0]);
			if (elems.size() > 1) sets.scrollSpeed.y = stof(elems[1]);
		}
		else if (arg == "shortcut")
			sets.shortcuts[key].key = SDL_GetScancodeFromName(val.c_str());
		else if (arg == "holder")
			sets.holders[key] = SDL_GetScancodeFromName(val.c_str());
	}
	return sets;
}

void Filer::SaveSettings(const ControlsSettings& sets) {
	vector<string> lines = {
		"scroll_speed=" + to_string(sets.scrollSpeed.x) + " " + to_string(sets.scrollSpeed.y)
	};
	for (const pair<string, Shortcut>& it : sets.shortcuts)
		lines.push_back("shortcut[" + it.first + "]=" + SDL_GetScancodeName(it.second.key));
	for (const pair<string, SDL_Scancode>& it : sets.holders)
		lines.push_back("holder[" + it.first + "]=" + SDL_GetScancodeName(it.second));
	WriteTextFile(dirSets() + "controls.ini", lines);
}

#ifdef __APPLE__
string Filer::execDir(bool raw) {
#else
string Filer::execDir() {
#endif
	const int MAX_LEN = 4096;
	fs::path path;
	
#ifdef _WIN32
	char buffer[MAX_LEN];
	if (GetModuleFileNameA(NULL, buffer, MAX_LEN))
		path = buffer;
	else {
		fs::path dir = fs::path(World::args[0]).parent_path();
		path = dir.is_absolute() ? dir : fs::initial_path().string() + "\\" + dir.string();
	}
#elif __APPLE__
	char buffer[MAX_LEN];
	uint size = sizeof(buffer);
	if (!_NSGetExecutablePath(buffer, &size))
		path = buffer;
	else
		path = fs::initial_path().string() + "/" + fs::path(World::args[0]).parent_path().string();

	if (!raw) {
		string test = path.string();
		size_t pos = 0;
		if (findString(test, ".app/", &pos))	// if running in a package
			for (size_t i=pos; i!=0; i--)
				if (test[i] == '/')
					return test.substr(0, i+1);
	}
#else
	char buffer[MAX_LEN];
	int len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
	if (len < MAX_LEN) {
		buffer[len] = '\0';
		path = buffer;
	}
	else
		path = fs::initial_path().string() + "/" + fs::path(World::args[0]).parent_path().string();

#endif
	return path.parent_path().string() + dsep;
}

string Filer::dirSets() {
#ifdef _WIN32
	return string(getenv("AppData")) + "\\VertiRead\\";
#elif __APPLE__
	return string(getenv("HOME")) + "/Library/Application Support/VertiRead/";
#else
	return string(getenv("HOME")) + "/.vertiread/";
#endif
}

string Filer::dirData() {
#ifdef __APPLE__
	return execDir(true) + "../Resources/";
#else
	return execDir();
#endif
}

string Filer::dirLangs() {
	return dirData() + "languages"+dsep;
}

string Filer::dirSnds() {
	return dirData() + "sounds"+dsep;
}

string Filer::dirTexs() {
	return dirData() + "textures"+dsep;
}

vector<fs::path> Filer::dirFonts() {
#ifdef _WIN32
	return {string(getenv("SystemDrive")) + "\\Windows\\Fonts\\"};
#elif __APPLE__
	return {string(getenv("HOME"))+"/Library/Fonts/", "/Library/Fonts/", "/System/Library/Fonts/", "/Network/Library/Fonts/"};
#else
	return {"/usr/share/fonts/"};
#endif
}

fs::path Filer::FindFont(const fs::path& font) {
	if (fs::path(font).is_absolute()) {	// check fontpath first
		if (fs::is_regular_file(font))
			return font;
		return CheckDirForFont(font.filename(), font.parent_path());
	}

	vector<fs::path> paths = dirFonts();	// check global font directories
	for (fs::path& dir : paths) {
		fs::path file = CheckDirForFont(font, dir);
		if (!file.empty())
			return file;
	}
	return "";
}

fs::path Filer::CheckDirForFont(const fs::path& font, const fs::path& dir) {
	for (fs::recursive_directory_iterator it(dir); it!=fs::recursive_directory_iterator(); it++)
		if (fs::is_regular_file(it->path())) {
			fs::path file = font.has_extension() ? it->path().filename() : delExt(it->path().filename());
			if (strcmpCI(file.string(), font.string()))
				return it->path();
		}
	return "";
}
