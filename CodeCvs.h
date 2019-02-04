#pragma once
#include <string>
#include <vector>
#include "cocos2d.h"
#include <string.h>

USING_NS_CC;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "../platform/android/jni/JniHelper.h"
#endif

namespace sqr{
	//下面这些函数我们不能放到module模块里面，因为返回的是string对象
	//会造成在dll里面分配但是在dll外面释放，创建和释放的new，delete不匹配
	// 将utf8转为unicode, pBuf为空返回装载Unicode字符的个数
	std::wstring& Utf8ToUCS2( std::wstring& dest, const std::string& src );
	std::wstring Utf8ToUCS2(const std::string& src);
	
	// 将Unicode转为utf8, pBuf为空返回装载char的字符个数
	std::string& UCS2ToUtf8( std::string& dest, const std::wstring& src );
	std::string UCS2ToUtf8(const std::wstring& src);

	/**
	\brief
		将utf8编码的unicode转换成utf16编码
	*/
	std::wstring& utf8_to_utf16(std::wstring& dest, const std::string& src);
	std::wstring  utf8_to_utf16(const std::string& src);

	/**
	\brief
		将utf16编码的unicode转换成utf8编码
	*/
	std::string& utf16_to_utf8(std::string& dest, const std::wstring& src);
	std::string  utf16_to_utf8(const std::wstring& src);

	void split_string(std::vector<std::string>& vec,
		const std::string& src, char sp);

	std::string join_string(const std::vector<std::string>& vec, const std::string& strAppend);
	std::string join_string(const std::vector<std::string>& vec,
		size_t begin_pos, size_t end_pos, const std::string& strAppend);


	int replace_all(std::string&, const std::string& from, const std::string& to);

	std::string makeString(int i);
	std::string makeString(int unsigned i);
	std::string makeString(char c);
	std::string makeString(char unsigned i);
	std::string makeString(short s);
	std::string makeString(short unsigned s);
    
    void RandIndex(std::vector<int>& vec, const std::vector<int>& vecSrc, int count);
    
    int strcmp_nocase(const std::string& s1, const std::string& s2);

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    jstring string2jstring(const std::string& str);
#endif
    
#ifdef WIN32
	char* wcharTochar(const wchar_t* wc);
	wchar_t* charTowchar(const char* c);
	
	std::string wstringTostring(const std::wstring& wc);
	std::wstring stringTowstring(const std::string& c);
#endif

    std::string commaNumString(int i);
}