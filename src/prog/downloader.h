#pragma once

#ifdef _BUILD_DOWNLOADERz
#include "utils/utils.h"
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <deque>

using std::deque;
using cofft = curl_off_t;

enum class DownloadState : uint8 {
	run,
	stop,
	skip
};

struct Comic {
	string title;
	vector<pairStr> chapters;	// name, url

	Comic(const string& title = emptyStr, const vector<pairStr>& chapters = {});
};

class WebSource {
public:
	enum Type : uint8 {
		MANGAHERE,
		MANGAMASTER,
		NHENTAI
	};
	static const array<string, NHENTAI+1> sourceNames, sourceUrls;

	CURL* curlMain;			// for downloading html
	SDL_mutex* mainLock;	// lock before using curlMain
	CURL* curlFile;			// handle for downloading files in dlQueue

public:
	WebSource(CURL* curlMain, SDL_mutex* mainLock, CURL* curlFile);
	virtual ~WebSource();

	const string& name() const;
	const string& baseUrl() const;
	virtual Type source() const = 0;

	virtual vector<pairStr> query(const string& text) = 0;
	virtual vector<pairStr> getChapters(const string& url) = 0;	// url needs to be of the comic's main info page; returned chapter lit has pairs of first name, secod url
	virtual DownloadState downloadPictures(const string& url, const string& drc) = 0;	// returns non-zero if interrupted

protected:
	xmlDoc* downloadHtml(const string& url);
	DownloadState downloadFile(const string& url, const string& drc);	// returns non-zero if progress got interrupted
	string toUrl(string str);

	static xmlNode* findElement(xmlNode* node, const char* tag);
	static vector<xmlNode*> findElements(xmlNode* node, const char* tag);
	static xmlNode* findElement(xmlNode* node, const char* tag, const char* attr, const char* val);
	static vector<xmlNode*> findElements(xmlNode* node, const char* tag, const char* attr, const char* val);
	static string getContent(xmlNode* node);
	static string getAttr(xmlNode* node, const char* attr);
	static bool hasAttr(xmlNode* node, const char* attr, const char* val);
	static bool namecmp(const xmlChar* name, const char* str);
};

inline const string& WebSource::name() const {
	return enumToStr(sourceNames, source());
}

inline const string& WebSource::baseUrl() const {
	return enumToStr(sourceUrls, source());
}

inline bool WebSource::namecmp(const xmlChar* name, const char* str) {
	return name && !strcmp(reinterpret_cast<const char*>(name), str);
}

class Mangahere : public WebSource {
public:
	Mangahere(CURL* curlMain, SDL_mutex* mainLock, CURL* curlFile);
	virtual ~Mangahere() override;

	virtual Type source() const override;
	virtual vector<pairStr> query(const string& text) override;
	virtual vector<pairStr> getChapters(const string& url) override;
	virtual DownloadState downloadPictures(const string& url, const string& drc) override;
};

inline Mangahere::Mangahere(CURL* curlMain, SDL_mutex* mainLock, CURL* curlFile) :
	WebSource(curlMain, mainLock, curlFile)
{}

inline WebSource::Type Mangahere::source() const {
	return MANGAHERE;
}

class Mangamaster : public WebSource {
public:
	Mangamaster(CURL* curlMain, SDL_mutex* mainLock, CURL* curlFile);
	virtual ~Mangamaster() override;

	virtual Type source() const override;
	virtual vector<pairStr> query(const string& text) override;
	virtual vector<pairStr> getChapters(const string& url) override;
	virtual DownloadState downloadPictures(const string& url, const string& drc) override;
};

inline Mangamaster::Mangamaster(CURL* curlMain, SDL_mutex* mainLock, CURL* curlFile) :
	WebSource(curlMain, mainLock, curlFile)
{}

inline WebSource::Type Mangamaster::source() const {
	return MANGAMASTER;
}

class Nhentai : public WebSource {
public:
	Nhentai(CURL* curlMain, SDL_mutex* mainLock, CURL* curlFile);
	virtual ~Nhentai() override;

	virtual Type source() const override;
	virtual vector<pairStr> query(const string& text) override;
	virtual vector<pairStr> getChapters(const string& url) override;
	virtual DownloadState downloadPictures(const string& url, const string& drc) override;

private:
	string getPictureUrl(const string& url);
};

inline Nhentai::Nhentai(CURL* curlMain, SDL_mutex* mainLock, CURL* curlFile) :
	WebSource(curlMain, mainLock, curlFile)
{}

inline WebSource::Type Nhentai::source() const {
	return NHENTAI;
}

class Downloader {
public:
	SDL_mutex* queueLock;	// lock when using dlQueue
private:
	uptr<WebSource> source;	// for downloading and interpreting htmls
	SDL_Thread* dlProc;		// thread for downloading files in dlQueue
	deque<Comic> dlQueue;	// current downloads
	vec2t dlProg;			// download progress of current comic
	DownloadState dlState;	// for controlling dlProc

public:
	Downloader();
	~Downloader();

	WebSource* getSource() const;
	void setSource(WebSource::Type type);
	const deque<Comic>& getDlQueue() const;
	DownloadState getDlState();
	const vec2t& getDlProg() const;
	bool downloadComic(const Comic& info);	// info.chapters needs to be filled with names and urls for getPictures
	bool startProc();
	void interruptProc();
	void finishProc();
	void deleteEntry(sizet id);
	void clearQueue();

private:
	static sizet writeText(char* ptr, sizet size, sizet nmemb, void* userdata);
	static int progress(void* clientp, cofft dltotal, cofft dlnow, cofft ultotal, cofft ulnow);
	static int downloadChaptersThread(void* data);
	DownloadState downloadChapters(vector<pairStr> chaps, const string& bdrc);	// returns non-zero if interrupted
};

inline WebSource* Downloader::getSource() const {
	return source.get();
}

inline const deque<Comic>& Downloader::getDlQueue() const {
	return dlQueue;
}

inline DownloadState Downloader::getDlState() {
	return dlState;
}

inline const vec2t& Downloader::getDlProg() const {
	return dlProg;
}

#else
class Downloader {
public:
	void interruptProc() {}
};
#endif