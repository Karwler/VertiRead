#pragma once

#include "utils/settings.h"
#include <fstream>

/* For interpreting lines in ini files:
   The first equal sign to be read splits the line into property and value, therefore titles can't contain equal signs.
   A key must be enclosed by brackets positioned before the equal sign.
   Any space characters before and after the property string as well as between a key's closing bracket and the equal sign are ignored.
   The same applies to titles and keys, but not to values. */
class IniLine {
public:
	enum class Type : uint8 {
		empty,
		prpVal,		// property, value, no key, no title
		prpKeyVal,	// property, key, value, no title
		title		// title, no everything else
	};
private:
	Type type = Type::empty;
	string prp;	// property, aka. the thing before the equal sign/brackets
	string key;	// the thing between the brackets (empty if there are no brackets)
	string val;	// value, aka. the thing after the equal sign

public:
	IniLine() = default;
	IniLine(const string& line);

	Type getType() const;
	const string& getPrp() const;
	const string& getKey() const;
	const string& getVal() const;

	void setVal(const string& property, const string& value);
	void setVal(const string& property, const string& vkey, const string& value);
	void setTitle(const string& title);
	Type setLine(const string& str);

	template <class... T> static void writeTitle(std::ofstream& ss, T&&... title);
	template <class P, class... T> static void writeVal(std::ofstream& ss, P&& prp, T&&... val);
	template <class P, class K, class... T> static void writeKeyVal(std::ofstream& ss, P&& prp, K&& key, T&&... val);
};

inline IniLine::IniLine(const string& line) {
	setLine(line);
}

inline IniLine::Type IniLine::getType() const {
	return type;
}

inline const string& IniLine::getPrp() const {
	return prp;
}

inline const string& IniLine::getKey() const {
	return key;
}

inline const string& IniLine::getVal() const {
	return val;
}

template <class... T>
void IniLine::writeTitle(std::ofstream& ss, T&&... title) {
	((ss << '[') << ... << std::forward<T>(title)) << "]\n";
}

template <class P, class... T>
void IniLine::writeVal(std::ofstream& ss, P&& prp, T&&... val) {
	((ss << std::forward<P>(prp) << '=') << ... << std::forward<T>(val)) << '\n';
}

template <class P, class K, class... T>
void IniLine::writeKeyVal(std::ofstream& ss, P&& prp, K&& key, T&&... val) {
	((ss << std::forward<P>(prp) << '[' << std::forward<K>(key) << "]=") << ... << std::forward<T>(val)) << '\n';
}

// handles all filesystem interactions
class FileSys {
public:
	static constexpr char extIni[] = ".ini";
private:
	static constexpr char defaultFrMode[] = "rb";
	static constexpr char defaultFwMode[] = "wb";
	static constexpr sizet archiveReadBlockSize = 10240;
#ifdef _WIN32
	static constexpr array<const char*, 22> takenFilenames = {
		"CON", "PRN", "AUX", "NUL",
		"COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
		"LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
	};
	static constexpr sizet drivesMax = 26;
#endif
	static constexpr char fileThemes[] = "themes.ini";
	static constexpr char fileSettings[] = "settings.ini";
	static constexpr char fileBindings[] = "bindings.ini";
	static constexpr char fileBooks[] = "books.dat";

	static constexpr char iniKeywordMaximized[] = "maximized";
	static constexpr char iniKeywordFullscreen[] = "fullscreen";
	static constexpr char iniKeywordResolution[] = "resolution";
	static constexpr char iniKeywordDirection[] = "direction";
	static constexpr char iniKeywordZoom[] = "zoom";
	static constexpr char iniKeywordSpacing[] = "spacing";
	static constexpr char iniKeywordPictureLimit[] = "picture_limit";
	static constexpr char iniKeywordFont[] = "font";
	static constexpr char iniKeywordTheme[] = "theme";
	static constexpr char iniKeywordShowHidden[] = "show_hidden";
	static constexpr char iniKeywordLibrary[] = "library";
	static constexpr char iniKeywordRenderer[] = "renderer";
	static constexpr char iniKeywordScrollSpeed[] = "scroll_speed";
	static constexpr char iniKeywordDeadzone[] = "deadzone";

	static constexpr char keyKey[] = "K_";
	static constexpr char keyButton[] = "B_";
	static constexpr char keyHat[] = "H_";
	static constexpr char keySep[] = "_";
	static constexpr char keyAxisPos[] = "A_+";
	static constexpr char keyAxisNeg[] = "A_-";
	static constexpr char keyGButton[] = "G_";
	static constexpr char keyGAxisPos[] = "X_+";
	static constexpr char keyGAxisNeg[] = "X_-";

	fs::path dirBase;	// application base directory
	fs::path dirSets;	// settings directory
	fs::path dirConfs;	// internal config directory
	array<fs::path, 2> dirFonts;	// os's font directories
public:
	FileSys();

	vector<string> getAvailableThemes() const;
	array<SDL_Color, Settings::defaultColors.size()> loadColors(const string& theme) const;	// updates settings' colors according to settings' theme
	bool getLastPage(const string& book, string& drc, string& fname) const;
	bool saveLastPage(const string& book, const string& drc, const string& fname) const;
	Settings* loadSettings() const;
	void saveSettings(const Settings* sets) const;
	array<Binding, Binding::names.size()> getBindings() const;
	void saveBindings(const array<Binding, Binding::names.size()>& bindings) const;
	fs::path findFont(const string& font) const;	// on success returns absolute path to font file, otherwise returns empty path

	static vector<fs::path> listDir(const fs::path& drc, bool files = true, bool dirs = true, bool showHidden = true);
	static pair<vector<fs::path>, vector<fs::path>> listDirSep(const fs::path& drc, bool showHidden = true);	// first is list of files, second is list of directories

	static fs::path validateFilename(const fs::path& file);
	static bool isPicture(const fs::path& file);
	static bool isFont(const fs::path& file);
	static bool isArchive(const fs::path& file);
	static bool isPictureArchive(const fs::path& file);
	static bool isArchivePicture(const fs::path& file, const string& pname);

	static archive* openArchive(const fs::path& file);
	static vector<string> listArchive(const fs::path& file);
	static mapFiles listArchivePictures(const fs::path& file, vector<string>& names);
	static SDL_Surface* loadArchivePicture(archive* arch, archive_entry* entry);

	static void moveContentThreaded(bool* running, fs::path src, fs::path dst);
	const fs::path& getDirSets() const;
	fs::path dirIcons() const;
	string windowIconPath() const;

private:
	static vector<string> readFileLines(const fs::path& file, bool printMessage = true);
	static string readTextFile(const fs::path& file, bool printMessage = true);
	static bool writeTextFile(const fs::path& file, const vector<string>& lines);
	static SDL_Color readColor(const string& line);

	template <sizet N> static fs::path searchFontDirs(const string& font, const array<fs::path, N>& dirs);
#ifdef _WIN32
	static vector<fs::path> listDrives();
#endif
};

inline const fs::path& FileSys::getDirSets() const {
	return dirSets;
}

inline fs::path FileSys::dirIcons() const {
	return dirConfs / "icons";
}

inline string FileSys::windowIconPath() const {
#if defined(__WIN32) || defined(APPIMAGE)
	return (dirBase / "vertiread.png").u8string();
#else
	return (dirConfs / "vertiread.png").u8string();
#endif
}
