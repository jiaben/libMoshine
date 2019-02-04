
#include "MSDownload.h"

#include <curl/curl.h>
#include <mutex>
#include <iostream>
#include <cocos2d.h>

USING_NS_CC;
using namespace std;

static mutex g_download_mutex;
static const int sDefaultTimeout = 30;
// 因为文件已经可能发生变化，所以不开启断点续传
static const bool sEnableContinue = false;

// 注：不要在下载线程内使用CCLOG，因为会引起线程阻塞

enum EDownloadStatus {
	OK,
	ERR_WRITE_FILE,
	ERR_NETWORK
};

class MSDownloadGuard {
public:
	MSDownloadGuard() {
		g_download_mutex.lock();
	}
	~MSDownloadGuard() {
		g_download_mutex.unlock();
	}
};

MSDownloadEntry::~MSDownloadEntry() {
	releaseUrl();
}

void MSDownloadEntry::releaseUrl() {
	if(curl) {
		curl_easy_cleanup(curl);
		curl = nullptr;
	}
}

void MSDownloadEntry::reset() {
	realsize = 0;
	downsize = 0;
	status = EDownloadStatus::OK;
}

bool createDirectoryForFile(const string& fullpath) {
    unsigned long found = fullpath.find_last_of("/\\");
    if (found != std::string::npos)
    {
        string parentDir = fullpath.substr(0, found+1);
        auto fu = FileUtils::getInstance();
        if(!fu->isDirectoryExist(parentDir)) {
            return fu->createDirectory(parentDir);
        }
        return true;
    }
    return false;
}

size_t downloadWrite(void *ptr, size_t size, size_t nmemb, void *userdata) {
	FILE *fp = (FILE*)userdata;
	size_t written = fwrite(ptr, size, nmemb, fp);
	return written;
}

int downloadProcess(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
	MSDownloadGuard g;

	auto dp = (MSDownloadEntry*)ptr;

	dp->realsize = totalToDownload;
	dp->downsize = nowDownloaded;
	
	return 0;
}

void threadFunc(MSDownloader *msdownloader) {
	auto de = msdownloader->startNextEntry();
	while(!msdownloader->isExit() && de) {
		de->curl = curl_easy_init();
		if(de->curl) {
			auto curl = de->curl;

			auto tempfilename = de->localpath;
			if(!createDirectoryForFile(tempfilename)) {
				de->status = EDownloadStatus::ERR_WRITE_FILE;
				msdownloader->finish(de);
				return;
			}

			// 如果不支持断点续传，则为wb
			string om = "wb";
			if (sEnableContinue) {
				om = "ab+";
			}

			FILE *fp = fopen(tempfilename.c_str(), om.c_str());
			if(!fp) {
				de->status = EDownloadStatus::ERR_WRITE_FILE;
				msdownloader->finish(de);
				return;
			}

			if(sEnableContinue) {
				// 移动后文件尾，支持断点续传
				if(fseek(fp, 0, SEEK_END) != 0) {
					de->status = EDownloadStatus::ERR_WRITE_FILE;
					msdownloader->finish(de);
					return;
				}
				auto len = ftell(fp);
				// TODO 是否需要判断是否支持断点续传呢？
				curl_easy_setopt(curl, CURLOPT_RESUME_FROM, len);
			}
			// 下载参数
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_URL, de->url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloadWrite);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, downloadProcess);
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, de);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, sDefaultTimeout);
			// 没有该选项时，会报 curl_resolv_timeout crash
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

			CURLcode res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				fclose(fp);
				de->status = EDownloadStatus::ERR_NETWORK;
				msdownloader->finish(de);
				return;
			}

			fclose(fp);

			// http reponse code
			long responseCode = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

			de->urlcode = responseCode;
			// result check
			if (responseCode != 200) {
				de->status = EDownloadStatus::ERR_NETWORK;
				msdownloader->finish(de);
				return;
			}

			de->status = 0;
			msdownloader->finish(de);
		} else {
			de->status = EDownloadStatus::ERR_NETWORK;
			msdownloader->finish(de);
		}
		de = msdownloader->startNextEntry();
	}
}

MSDownloader::~MSDownloader() {
	MSDownloadGuard g;
	_isexit = true;
	for(auto &t : _desList) {
		t->release();
	}
	for(auto &t : _desLoad) {
		t->release();
	}
    for(auto &t : _desFinish) {
        t->release();
    }
}

bool MSDownloader::init() {
	_isexit = false;
	_threadNum = 0;
	return true;
}

void MSDownloader::add(const string& filename, const string& url, const string& localpath, size_t size/*=0*/) {
	MSDownloadGuard g;

	// 下载信息
	auto de = new MSDownloadEntry();
	de->autorelease();
	de->filename = filename;
	de->url = url;
	de->localpath = localpath;
	de->downloader = this;
	de->filesize = size;
	de->curl = nullptr;
	de->status = EDownloadStatus::OK;

	de->retain();
	_desList.push_back(de);
}

void MSDownloader::start(int threadNum) {
	_threadNum = threadNum;
	for(int i = 0; i < threadNum; i++) {
		thread t(threadFunc, this);
		t.detach();
	}
}

MSDownloadEntry* MSDownloader::startNextEntry() {
	MSDownloadGuard g;

	if(_desList.size() > 0) {
		auto de = _desList.front();
		_desLoad.push_back(de);
		_desList.pop_front();
		return de;
	}
	return nullptr;
}

void MSDownloader::finish(MSDownloadEntry *de) {
    MSDownloadGuard g;

	de->releaseUrl();
    
    for(auto iter = _desLoad.begin(); iter != _desLoad.end(); iter++) {
        if(*iter == de) {
            _desLoad.erase(iter);
            _desFinish.push_back(de);
            break;
        }
    }
}

void MSDownloader::remove(MSDownloadEntry *de) {
	MSDownloadGuard g;

	// 只检查已完成队列
	for(auto iter = _desFinish.begin(); iter != _desFinish.end(); iter++) {
		if(*iter == de) {
			_desFinish.erase(iter);
			//de->release();
			break;
		}
	}
}
