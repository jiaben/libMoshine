#include "CodeCvs.h"

#if defined(__FLASCC__)
#include <iconv.h>
#endif

using namespace std;

namespace sqr{

	wstring& Utf8ToUCS2( wstring& dest, const string& src )
	{
		dest = L"";

		for( char* pBuf = (char*)(&src[0]); *pBuf; )
		{		
			wchar_t cUCS2;
			if( ( ( pBuf[0] )&0x80 ) == 0 )
			{
				cUCS2 = pBuf[0];
				pBuf++;
			}
			else if( ( ( pBuf[0] )&0xE0 ) == 0xC0 )
			{
				cUCS2 = ( ( ( pBuf[0] )&0x1F )<<6 )|( pBuf[1]&0x3F );
				pBuf += 2;
			}
			else
			{
				cUCS2 = ( ( ( pBuf[0] )&0x0F )<<12 )|( ( pBuf[1]&0x3F ) << 6 )|( pBuf[2]&0x3F );
				pBuf += 3;
			}

			dest.append(1, cUCS2);
		}

		return dest;
	}

	wstring Utf8ToUCS2(const string& src)
	{
		wstring dest;
		return Utf8ToUCS2(dest, src);
	}


	string& UCS2ToUtf8( string& dest, const wstring& src )
	{
		dest = "";

		for ( size_t i=0; i<src.length(); ++i)
		{
			wchar_t cUCS2 = src[i];
			char	cUTF8 = 0;
			if( cUCS2 < 0x80 )
			{
				cUTF8 = (char)cUCS2;
				dest.append(1, cUTF8);
			}
			else if( cUCS2 < 0x07FF )
			{
				cUTF8			= ( cUCS2 >> 6 )|0xC0;
				dest.append(1, cUTF8);
				cUTF8			= ( cUCS2&0x3f )|0x80;
				dest.append(1, cUTF8);
			}
			else
			{
				cUTF8			= ( cUCS2 >> 12 )|0xE0;
				dest.append(1, cUTF8);
				cUTF8			= ( ( cUCS2 >> 6 )&0x3f )|0x80;
				dest.append(1, cUTF8);
				cUTF8			= ( cUCS2&0x3f )|0x80;
				dest.append(1, cUTF8);
			}
		}

		return dest;
	}

	string UCS2ToUtf8(const wstring& src)
	{
		string dest;
		return UCS2ToUtf8(dest, src);
	}


	wstring& utf8_to_utf16(wstring& dest, const string& src)
	{
		Utf8ToUCS2(dest, src);
		return dest;
	}

	wstring utf8_to_utf16(const string& src)
	{
		return Utf8ToUCS2(src);
	}

	string& utf16_to_utf8( string& dest, const wstring& src )
	{
		UCS2ToUtf8(dest, src);
		return dest;
	}

	string utf16_to_utf8(const wstring& src)
	{
		return UCS2ToUtf8(src);
	}
	
	void split_string(std::vector<string>& vec,
		const std::string& src, char sp)
	{
		vec.clear();
		size_t index = 0;
		size_t pos = 0;
		while((pos = src.find(sp, index)) != string::npos)
		{
			vec.push_back(src.substr(index, pos-index));
			index = pos+1;
		}
		vec.push_back(src.substr(index));
	}

	int replace_all(std::string& str,  
		const std::string& pattern,  
		const std::string& newpat) 
	{ 
		int count = 0; 
		const size_t nsize = newpat.size(); 
		const size_t psize = pattern.size(); 

		for(size_t pos = str.find(pattern, 0);  
			pos != std::string::npos; 
			pos = str.find(pattern,pos + nsize)) 
		{ 
			str.replace(pos, psize, newpat); 
			count++; 
		} 

		return count; 
	} 

	std::string makeString(int i){
		char ch[12];
		sprintf(ch, "%d", i);
		return ch;
	}
	std::string makeString(int unsigned i){
		char ch[12];
		sprintf(ch, "%u", i);
		return ch;
	}
	std::string makeString(short s){
		return makeString((int)s);
	}
	std::string makeString(short unsigned s){
		return makeString((int unsigned)s);
	}
	std::string makeString(char c){
		return makeString((int)c);
	}
	std::string makeString(char unsigned c){
		return makeString((int unsigned)c);
	}

	std::string join_string(const std::vector<std::string>& vec,
		const std::string& strAppend)
	{
		string result = "";
		bool flag = false;
		for (size_t i = 0; i < vec.size(); ++i){
			if(flag)
				result.append(strAppend);
			flag = true;
			result.append(vec[i]);
		}
		return result;
	}

	std::string join_string(const std::vector<std::string>& vec,
		size_t begin_pos, size_t end_pos, 
		const std::string& strAppend)
	{
		string result = "";
		bool flag = false;
		size_t endp = min<size_t>(vec.size(), end_pos);
		for (size_t i = begin_pos; i < endp; ++i){
			if(flag)
				result.append(strAppend);
			flag = true;
			result.append(vec[i]);
		}
		return result;
	}

    void RandIndex(std::vector<int>& vec, const std::vector<int>& vecSrc, int count)
    {
        vector<int> deqSrc;
        size_t sz = vecSrc.size();
        deqSrc.resize(vecSrc.size());
        for (size_t i = 0; i < sz; ++i){
            deqSrc[i] = vecSrc[i];
        }
        
        for (size_t j = 0; j < count && j < sz; ++j){
            int i = rand() % (deqSrc.size()-j);
            vec.push_back(deqSrc[i]);
            deqSrc[i] = deqSrc[sz-j-1];
        }
    }
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    jstring string2jstring(const std::string& str) {
        JNIEnv *env = NULL;
        cocos2d::JniHelper::getJavaVM()->GetEnv((void**)&env, JNI_VERSION_1_4);
        
        jclass strClass = env->FindClass("java/lang/String");
        jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
        jbyteArray bytes = env->NewByteArray(str.length());
        env->SetByteArrayRegion(bytes, 0, str.length(), (jbyte*)str.c_str());
        jstring encoding = env->NewStringUTF("utf-8");
        return (jstring)env->NewObject(strClass, ctorID, bytes, encoding);
    }
#endif
    
    int strcmp_nocase(const string& s1, const string& s2) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
        return stricmp(s1.c_str(), s2.c_str());
#else
        return strcasecmp(s1.c_str(), s2.c_str());
#endif
    }
    
    std::string commaNumString(int i) {
        string result = "";
        string s = makeString(i);
        s.reserve();
        int idx = 1;
        for (size_t i = 0; i < s.length(); i++) {
            if (i % 4 == 0) {
                result += ",";
                idx = 1;
            } else {
                idx++;
            }
            result += s.at(i);
        }
        result.reserve();
        return result;
    }

#ifdef WIN32
	char* wcharTochar(const wchar_t* wc) {
		int len= WideCharToMultiByte(CP_ACP,0,wc,wcslen(wc),NULL,0,NULL,NULL);  
		char *c=new char[len+1];  
		WideCharToMultiByte(CP_ACP,0,wc,wcslen(wc),c,len,NULL,NULL);  
		c[len]='\0';  
		return c; 
	}
	
	wchar_t* charTowchar(const char* c) {
		int len = MultiByteToWideChar(CP_ACP,0,c,strlen(c),NULL,0);  
		wchar_t *wc=new wchar_t[len+1];  
		MultiByteToWideChar(CP_ACP,0,c,strlen(c),wc,len);  
		wc[len]='\0';  
		return wc;
	}
		
	std::string wstringTostring(const wstring& wc) {
		char *c = wcharTochar(wc.c_str());
		string str = c;
		delete c;
		return str;
	}

	std::wstring stringTowstring(const string& c) {
		wchar_t *wc = charTowchar(c.c_str());
		wstring str = wc;
		delete wc;
		return str;
	}
#endif

}
