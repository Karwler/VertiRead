#include "filer.h"
#include "engine/world.h"

EDirFilter operator|(EDirFilter a, EDirFilter b) {
	return static_cast<EDirFilter>(static_cast<byte>(a) | static_cast<byte>(b));
}

int Filer::CheckDirectories() {
	int retval = 0;
	if (!fs::exists(dirPlist()))
		fs::create_directory(dirPlist());
	if (!fs::exists(dirSets()))
		fs::create_directory(dirSets());
	if (!fs::exists(dirSnds())) {
		cerr << "couldn't find sound directory" << endl;
		retval = 1;
	}
	if (!fs::exists(dirTexs())) {
		cerr << "couldn't find texture directory" << endl;
		retval = 2;
	}
	return retval;
}

bool Filer::ReadTextFile(string file, vector<string>& lines) {
	ifstream ifs(file.c_str());
	if (!ifs.good()) {
		cerr << "couldn't read file " << file << endl;
		return false;
	}
	lines.clear();
	for (string line; getline(ifs, line);)
		lines.push_back(line);
	return true;
}

bool Filer::WriteTextFile(string file, const vector<string>& lines) {
	ofstream ofs(file.c_str());
	if (!ofs.good()) {
		cerr << "couldn't write file " << file << endl;
		return false;
	}
	for (const string& line : lines)
		ofs << line << endl;
	return true;
}

vector<fs::path> Filer::ListDir(fs::path dir, EDirFilter filter, vector<string> extFilter) {
	vector<fs::path> names;
	if (!fs::exists(dir))
		return names;
	for (fs::directory_iterator it(dir); it != fs::directory_iterator(); it++) {
		if (filter == 0)
			names.push_back(it->path());
		else if (filter & FILTER_FILE && fs::is_regular_file(it->path())) {
			if (extFilter.empty())
				names.push_back(it->path());
			else for (string& ext : extFilter)
				if (it->path().extension() == ext) {
					names.push_back(it->path());
					break;
				}
		}
		else if (filter & FILTER_DIR && fs::is_directory(it->path()))
			names.push_back(it->path());
		else if (filter & FILTER_LINK && fs::is_symlink(it->path()))
			names.push_back(it->path());
	}
	return names;
}

Playlist Filer::LoadPlaylist(string name) {
	vector<string> lines;
	if (!ReadTextFile(dirPlist() + name, lines))
		return Playlist();

	Playlist plist(name);
	for (string& line : lines) {
		string arg, val;
		SplitIniLine(line, &arg, &val);
		if (arg == "file")
			plist.songs.push_back(val);
		else if (arg == "book")
			plist.books.push_back(val);
	}
	return plist;
}

void Filer::SavePlaylist(Playlist plist) {
	vector<string> lines;
	for (string& file : plist.songs)
		lines.push_back("file=" + file);
	for (string& name : plist.books)
		lines.push_back("book=" + name);
	WriteTextFile(dirPlist() + plist.name + ".txt", lines);
}

GeneralSettings Filer::LoadGeneralSettings() {
	vector<string> lines;
	if (!ReadTextFile(dirSets() + "general.ini", lines))
		return GeneralSettings();

	GeneralSettings sets;
	for (string& line : lines) {
		string arg, val;
		SplitIniLine(line, &arg, &val);
		// load settings
	}
	return sets;
}

void Filer::SaveSettings(GeneralSettings sets) {
	vector<string> lines;
	// save settings
	WriteTextFile(dirSets() + "general.ini", lines);
}

VideoSettings Filer::LoadVideoSettings() {
	vector<string> lines;
	if (!ReadTextFile(dirSets() + "video.ini", lines))
		return VideoSettings();

	VideoSettings sets;
	for (string& line : lines) {
		string arg, val, key;
		SplitIniLine(line, &arg, &val, &key);
		if (arg == "font")
			sets.font = val;
		else if (arg == "vsync")
			sets.vsync = stob(val);
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

void Filer::SaveSettings(VideoSettings sets) {
	vector<string> lines;
	lines.push_back("font=" + sets.font);
	lines.push_back("vsync=" + btos(sets.vsync));
	lines.push_back("renderer=" + sets.renderer);
	lines.push_back("maximized=" + btos(sets.maximized));
	lines.push_back("fullscreen=" + btos(sets.fullscreen));
	lines.push_back("resolution=" + to_string(sets.resolution.x) + ' ' + to_string(sets.resolution.y));
	for (const pair<EColor, vec4b>& it : sets.colors)
		lines.push_back("color["+to_string(int(it.first))+"]=" + to_string(it.second.x) + ' ' + to_string(it.second.y) + ' ' + to_string(it.second.z) + ' ' + to_string(it.second.a));
	WriteTextFile(dirSets() + "video.ini", lines);
}

AudioSettings Filer::LoadAudioSettings() {
	vector<string> lines;
	if (!ReadTextFile(dirSets() + "audio.ini", lines))
		return AudioSettings();

	AudioSettings sets;
	for (string& line : lines) {
		string arg, val;
		SplitIniLine(line, &arg, &val);
		if (arg == "music_vol")
			sets.musicVolume = stoi(val);
		else if (arg == "interface_vol")
			sets.soundVolume = stoi(val);
		else if (arg == "song_delay")
			sets.songDelay = stof(val);
	}
	return sets;
}

void Filer::SaveSettings(AudioSettings sets) {
	vector<string> lines;
	lines.push_back("music_vol=" + to_string(sets.musicVolume));
	lines.push_back("interface_vol=" + to_string(sets.soundVolume));
	lines.push_back("song_delay=" + to_string(sets.songDelay));
	WriteTextFile(dirSets() + "audio.ini", lines);
}

ControlsSettings Filer::LoadControlsSettings() {
	vector<string> lines;
	if (!ReadTextFile(dirSets() + "controls.ini", lines))
		return ControlsSettings();

	ControlsSettings sets(false);
	for (string& line : lines) {
		string arg, val, key;
		SplitIniLine(line, &arg, &val, &key);
		if (arg == "shortcut") {
			Shortcut* it = sets.shortcut(key);
			if (!it) {
				sets.shortcuts.push_back(Shortcut(key, false));
				it = &sets.shortcuts[sets.shortcuts.size() - 1];
			}
			for (string word : getWords(val))
				it->keys.push_back(stok(word));
		}
	}
	sets.FillMissingBindings();
	return sets;
}

void Filer::SaveSettings(ControlsSettings sets) {
	vector<string> lines;
	for (Shortcut& it : sets.shortcuts)
		for (SDL_Keysym& key : it.keys)
			lines.push_back("shortcut[" + it.Name() + "]=" + ktos(key));
	WriteTextFile(dirSets() + "controls.ini", lines);
}

string Filer::execDir() {
	fs::path path;
#ifdef _WIN32
	TCHAR buffer[2048];
	GetModuleFileName (NULL, buffer, 2048);
	for (uint i = 0; buffer[i] != 0; i++)
		path += buffer[i];
#else
	char buffer[PATH_MAX];
	int len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
	buffer[len] = '\0';
	path = buffer;
#endif
	return path.parent_path().string() + dsep;
}

string Filer::dirLib() {
	return execDir() + "library" + dsep;
}

string Filer::dirPlist() {
	return execDir() + "playlists" + dsep;
}

string Filer::dirSets() {
	return execDir() + "settings" + dsep;
}

string Filer::dirSnds() {
	return dirLib() + "sounds" + dsep;
}

string Filer::dirTexs() {
	return dirLib() + "textures" + dsep;
}
