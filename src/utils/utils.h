#pragma once

// stuff that's used pretty much everywhere
#include "utils/vec2.h"
#include <SDL2/SDL_image.h>
#include <array>
#include <climits>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#ifndef _WIN32
#include <strings.h>
#endif

// get rid of SDL's main
#ifdef main
#undef main
#endif

// to make life easier
using std::array;
using std::pair;
using std::vector;

using int8 = int8_t;
using uint8 = uint8_t;
using int16 = int16_t;
using uint16 = uint16_t;
using int32 = int32_t;
using uint32 = uint32_t;
using int64 = int64_t;
using uint64 = uint64_t;

template <class... T> using umap = std::unordered_map<T...>;
template <class... T> using uptr = std::unique_ptr<T...>;
template <class... T> using uset = std::unordered_set<T...>;

using sizet = size_t;
using pdift = ptrdiff_t;
using pairStr = pair<string, string>;

using vec2i = vec2<int>;
using vec2f = vec2<float>;
using vec2t = vec2<sizet>;

// forward declaraions
class Button;
class Layout;
class Program;
class ProgState;

// events
using PCall = void (Program::*)(Button*);
using SBCall = void (ProgState::*)();
using SACall = void (ProgState::*)(float);

// global constants
#ifdef _WIN32
const char dsep = '\\';
const char dseps[] = "\\";
#else
const char dsep = '/';
const char dseps[] = "/";
#endif
const string emptyStr = "";
const string invalidStr = "invalid";

// general wrappers

enum class UserCode : int32 {
	readerProgress,
	readerFinished,
	downloadProgress,
	downloadNext,
	downlaodFinished,
	moveProgress,
	moveFinished
};

inline vec2i texSize(SDL_Texture* tex) {
	vec2i s;
	SDL_QueryTexture(tex, nullptr, nullptr, &s.x, &s.y);
	return s;
}

inline vec2i mousePos() {
	vec2i p;
	SDL_GetMouseState(&p.x, &p.y);
	return p;
}

inline string getRendererName(int id) {
	SDL_RendererInfo info;
	SDL_GetRenderDriverInfo(id, &info);
	return info.name;
}

vector<string> getAvailibleRenderers();

// SDL_Rect wrapper

struct Rect : SDL_Rect {
	Rect() = default;
	constexpr Rect(int n);
	constexpr Rect(int x, int y, int w, int h);
	constexpr Rect(const vec2i& pos, const vec2i& size);

	vec2i& pos();
	constexpr vec2i pos() const;
	vec2i& size();
	constexpr vec2i size() const;
	constexpr vec2i end() const;

	bool contain(const vec2i& point) const;
	Rect crop(const Rect& frame);			// crop rect so it fits in the frame (aka set rect to the area where they overlap) and return how much was cut off
	Rect intersect(const Rect& rect) const;	// same as above except it returns the overlap instead of the crop and it doesn't modify itself
};

inline constexpr Rect::Rect(int n) :
	SDL_Rect({n, n, n, n})
{}

inline constexpr Rect::Rect(int x, int y, int w, int h) :
	SDL_Rect({x, y, w, h})
{}

inline constexpr Rect::Rect(const vec2i& pos, const vec2i& size) :
	SDL_Rect({pos.x, pos.y, size.w, size.h})
{}

inline vec2i& Rect::pos() {
	return *reinterpret_cast<vec2i*>(this);
}

inline constexpr vec2i Rect::pos() const {
	return vec2i(x, y);
}

inline vec2i& Rect::size() {
	return reinterpret_cast<vec2i*>(this)[1];
}

inline constexpr vec2i Rect::size() const {
	return vec2i(w, h);
}

inline constexpr vec2i Rect::end() const {
	return pos() + size();
}

inline bool Rect::contain(const vec2i& point) const {
	return SDL_PointInRect(reinterpret_cast<const SDL_Point*>(&point), this);
}

inline Rect Rect::intersect(const Rect& rect) const {
	Rect isct;
	return SDL_IntersectRect(this, &rect, &isct) ? isct : Rect(0);
}

// SDL_Texture wrapper

struct Texture {
	string name;
	SDL_Texture* tex;

	Texture(const string& name = emptyStr, SDL_Texture* tex = nullptr);

	vec2i res() const;
};

inline Texture::Texture(const string& name, SDL_Texture* tex) :
	name(name),
	tex(tex)
{}

inline vec2i Texture::res() const {
	return texSize(tex);
}

// SDL_Thread wrapper

class Thread {
public:
	Thread(int (*func)(void*), void* data = nullptr);
	~Thread();

	bool start(int (*func)(void*));
	bool getRun() const;
	void interrupt();
	int finish();

	void* data;
private:
	SDL_Thread* proc;
	bool run;
};

inline Thread::~Thread() {
	finish();
}

inline bool Thread::getRun() const {
	return run;
}

inline void Thread::interrupt() {
	run = false;
}

// files and strings

int strnatcmp(const char* a, const char* b);	// natural string compare

inline bool strnatless(const string& a, const string& b) {
	return strnatcmp(a.c_str(), b.c_str()) < 0;
}

inline int strcicmp(const string& a, const string& b) {	// case insensitive check if strings are equal
#ifdef _WIN32
	return _stricmp(a.c_str(), b.c_str());
#else
	return strcasecmp(a.c_str(), b.c_str());
#endif
}

inline int strncicmp(const string& a, const string& b, sizet n) {	// case insensitive check if strings are equal
#ifdef _WIN32
	return _strnicmp(a.c_str(), b.c_str(), n);
#else
	return strncasecmp(a.c_str(), b.c_str(), n);
#endif
}
#ifdef _WIN32
inline bool isDriveLetter(const string& path) {
	return path.length() >= 2 && ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) && path[1] == ':' && std::all_of(path.begin() + 2, path.end(), [](char c) -> bool { return c == dsep; });
}

inline bool isDriveLetter(char c) {
	return c >= 'A' && c <= 'Z';
}
#endif
bool pathCmp(const string& as, const string& bs);
bool isSubpath(const string& path, string parent);
string parentPath(const string& path);
string getChild(const string& path, const string& parent);
string filename(const string& path);	// get filename from path
string strEnclose(string str);
vector<string> strUnenclose(const string& str);

inline bool isSpace(char c) {
	return c > '\0' && c <= ' ';
}

inline bool notSpace(char c) {
	return c <= '\0' || c > ' ';
}

inline bool isDigit(char c) {
	return c >= '0' && c <= '9';
}

inline bool notDigit(char c) {
	return c < '0' || c > '9';
}

inline string trim(const string& str) {
	string::const_iterator pos = std::find_if(str.begin(), str.end(), [](char c) -> bool { return uchar(c) > ' '; });
	return string(pos, std::find_if(str.rbegin(), std::make_reverse_iterator(pos), [](char c) -> bool { return uchar(c) > ' '; }).base());
}

inline string trimZero(const string& str) {
	sizet id = str.find_last_not_of('0');
	return str.substr(0, str[id] == '.' ? id : id + 1);
}

inline string getExt(const string& path) {
	string::const_reverse_iterator it = std::find_if(path.rbegin(), path.rend(), [](char c) -> bool { return c == '.' || c == dsep; });
	return it != path.rend() && *it == '.' ? string(it.base(), path.end()) : emptyStr;
}

inline bool hasExt(const string& path) {
	string::const_reverse_iterator it = std::find_if(path.rbegin(), path.rend(), [](char c) -> bool { return c == '.' || c == dsep; });
	return it != path.rend() && *it == '.';
}

inline string delExt(const string& path) {
	string::const_reverse_iterator it = std::find_if(path.rbegin(), path.rend(), [](char c) -> bool { return c == '.' || c == dsep; });
	return it != path.rend() && *it == '.' ? string(path.begin(), it.base() - 1) : emptyStr;
}

inline string appDsep(const string& path) {
	return path.size() && path.back() == dsep ? path : path + dsep;
}
#ifdef _WIN32
inline wstring appDsep(const wstring& path) {
	return path.size() && path.back() == dsep ? path : path + wchar(dsep);
}
#endif
inline string childPath(const string& parent, const string& child) {
#ifdef _WIN32
	return std::all_of(parent.begin(), parent.end(), [](char c) -> bool { return c == dsep; }) && isDriveLetter(child) ? child : appDsep(parent) + child;
#else
	return appDsep(parent) + child;
#endif
}

template <class T>
bool isDotName(const T& str) {
	return str[0] == '.' && (str[1] == '\0' || (str[1] == '.' && str[2] == '\0'));
}

// geometry?

template <class T>
bool outRange(const T& val, const T& min, const T& max) {
	return val < min || val > max;
}

template <class T>
const T& clampHigh(const T& val, const T& max) {
	return (val > max) ? max : val;
}

template <class T>
vec2<T> clampHigh(const vec2<T>& val, const vec2<T>& max) {
	return vec2<T>(clampHigh(val.x, max.x), clampHigh(val.y, max.y));
}

template <class T>
const T& clampLow(const T& val, const T& min) {
	return (val < min) ? min : val;
}

template <class T>
vec2<T> clampLow(const vec2<T>& val, const vec2<T>& min) {
	return vec2<T>(clampLow(val.x, min.x), clampLow(val.y, min.y));
}

// conversions
#ifdef _WIN32
string wtos(const wchar* wstr);
string wtos(const wstring& wstr);
wstring stow(const char* str);
wstring stow(const string& str);
#endif
inline bool stob(const string& str) {
	return str == "true" || str == "1";
}

inline string btos(bool b) {
	return b ? "true" : "false";
}

template <class T, sizet N>
T strToEnum(const array<string, N>& names, const string& str) {
	return T(std::find_if(names.begin(), names.end(), [str](const string& it) -> bool { return !strcicmp(it, str); }) - names.begin());
}

template <class T>
T strToVal(const umap<T, string>& names, const string& str) {
	umap<uint8, string>::const_iterator it = std::find_if(names.begin(), names.end(), [str](const pair<T, string>& it) -> bool { return !strcicmp(it.second, str); });
	return it != names.end() ? it->first : T(0);
}

inline long sstol(const string& str, int base = 0) {
	return strtol(str.c_str(), nullptr, base);
}

inline llong sstoll(const string& str, int base = 0) {
	return strtoll(str.c_str(), nullptr, base);
}

inline ulong sstoul(const string& str, int base = 0) {
	return strtoul(str.c_str(), nullptr, base);
}

inline ullong sstoull(const string& str, int base = 0) {
	return strtoull(str.c_str(), nullptr, base);
}

inline float sstof(const string& str) {
	return strtof(str.c_str(), nullptr);
}

inline double sstod(const string& str) {
	return strtod(str.c_str(), nullptr);
}

inline ldouble sstold(const string& str) {
	return strtold(str.c_str(), nullptr);
}

// container stuff

template <class T, class P, class F, class... A>
bool foreachFAround(const vector<T>& vec, typename vector<T>::const_iterator start, P* parent, F func, A... args) {
	if (std::find_if(start + 1, vec.end(), [parent, func, args...](const T& it) -> bool { return (parent->*func)(it, args...); }) != vec.end())
		return true;
	return std::find_if(vec.begin(), start, [parent, func, args...](const T& it) -> bool { return (parent->*func)(it, args...); }) != start;
}

template <class T, class P, class F, class... A>
bool foreachRAround(const vector<T>& vec, typename vector<T>::const_reverse_iterator start, P* parent, F func, A... args) {
	if (std::find_if(start + 1, vec.rend(), [parent, func, args...](const T& it) -> bool { return (parent->*func)(it, args...); }) != vec.rend())
		return true;
	return vec.size() && std::find_if(vec.rbegin(), start, [parent, func, args...](const T& it) -> bool { return (parent->*func)(it, args...); }) != start;
}

template <class T, class P, class F, class... A>
bool foreachAround(const vector<T>& vec, typename vector<T>::const_iterator start, bool fwd, P* parent, F func, A... args) {
	return fwd ? foreachFAround(vec, start, parent, func, args...) : foreachRAround(vec, std::make_reverse_iterator(start + 1), parent, func, args...);
}

template <class T>
T popBack(vector<T>& vec) {
	T t = vec.back();
	vec.pop_back();
	return t;
}

template <class T>
void clear(vector<T*>& vec) {
	for (T* it : vec)
		delete it;
	vec.clear();
}

inline void clearTexVec(vector<Texture>& vec) {
	for (Texture& it : vec)
		SDL_DestroyTexture(it.tex);
	vec.clear();
}
