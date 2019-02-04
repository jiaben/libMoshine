
#ifndef _MOSHINE_DOWNLOAD_H
#define _MOSHINE_DOWNLOAD_H

#include "cocos2d.h"
#include <string>
#include <list>
#include <curl/curl.h>
#include <thread>

USING_NS_CC;
using namespace std;

class MSDownloader;

class MSDownloadEntry : public Ref {
public:
	string filename;
	string url;
	string localpath;

	double filesize;	// 不一定准确的大小
	double realsize;	// curl返回的大小
	double downsize;	// 已下载的大小

	CURL* curl;
	MSDownloader* downloader;

	int status;
	int urlcode;

	~MSDownloadEntry();

	inline const string& getFilename() const {return filename;};
	inline const string& getUrl() const {return url;};
	inline const string& getLocalpath() const {return localpath;};
	inline double getFilesize() const {return filesize;};
	inline double getRealsize() const {return realsize;};
	inline double getDownsize() const {return downsize;};
	inline int getStatus() const {return status;};
	inline int getUrlCode() const {return urlcode;};
	void releaseUrl();

	void reset();
};

class MSDownloader : public Ref {
public:
	CREATE_FUNC(MSDownloader);
	~MSDownloader();

	bool init();

	void add(const string& filename, const string& url, const string& localpath, size_t size=0);
	void start(int threadNum);
	
	void remove(MSDownloadEntry *de);
	void finish(MSDownloadEntry *de);
	MSDownloadEntry* startNextEntry();

	list<MSDownloadEntry*> getEntries(){return _desLoad;};
	list<MSDownloadEntry*> getEntriesFinish(){return _desFinish;};
	bool isExit() const {return _isexit;}
private:
	bool _isexit;
	int _threadNum;
	list<MSDownloadEntry*> _desList;	// 待下载列表
	list<MSDownloadEntry*> _desLoad;	// 下载列表
    list<MSDownloadEntry*> _desFinish; // 已结束队列
};

#endif // !_MOSHINE_UPDATE_H
