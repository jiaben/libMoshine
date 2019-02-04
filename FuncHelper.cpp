#include "DeviceHelper.h"
#include "cocos2d.h"
#include "CodeCvs.h"
#include "ByteBuffer.h"
#include <zlib.h>
#include <string.h>
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "../platform/android/jni/JniHelper.h"
#elif(CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#import "../ios/OcFuncUtils.h"
#endif

void split(string& str, string& delim, vector<string>* ret)
{
    size_t last = 0;
    size_t index = str.find_first_of(delim,last);
    while (index != string::npos)
    {
        ret->push_back(str.substr(last,index-last));
        last=index+1;
        index=str.find_first_of(delim,last);
    }
    if (index-last>0)
    {
        ret->push_back(str.substr(last,index-last));
    }
}

string doCommFunc(string funcName, string arg_str)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    //vector<string> arg_vec;
    //string delim = ",";
    //split(arg_str,delim,&arg_vec);
    if (funcName == "getDeviceID")
        return getDeviceUniqueIdentifier();
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	cocos2d::JniMethodInfo pInfo;
	if(cocos2d::JniHelper::getStaticMethodInfo(pInfo, "com/ms/comm/JniFuncUtils", "doFunc", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;")){
		jstring ret = (jstring)pInfo.env->CallStaticObjectMethod(pInfo.classID, pInfo.methodID, pInfo.env->NewStringUTF(funcName.c_str()), pInfo.env->NewStringUTF(arg_str.c_str()));
		return cocos2d::JniHelper::jstring2string(ret);
	}
    return "";
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    if (funcName == "getDeviceID")
	{
		string macStr;
		getWinMacAddress(macStr);
		return macStr;
	}
#else
	return "undefined device";
#endif
    CCLOG("doCommFunc Error Func not exist %s", funcName.c_str());
    return "doCommFunc Error Func not exist";
}

void dispatchCommFuncEvent(string event_name, string result_str)
{
	auto eventDispatcher = Director::getInstance()->getEventDispatcher();
	ByteBuffer buffer;
	buffer.Write<uint8>(result_str.length());
	buffer.Write((uint8*)result_str.c_str(), result_str.length());
	eventDispatcher->dispatchCustomEvent(event_name, &buffer);
}

string doCommFuncByEvent(string funcName, string arg_str, string event_name)
{
	//doFunc by funcName { return result_str }
	if (event_name == "")
		return "";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    NSString * strFuncName = [NSString stringWithUTF8String:funcName.c_str()];
    NSString * strArgStr = [NSString stringWithUTF8String:arg_str.c_str()];
    NSString * strEventName = [NSString stringWithUTF8String:event_name.c_str()];
    [[OcFuncUtils sharedInstanceMethod] doHandlerFuncByEvent:strFuncName argStr:strArgStr event_name:strEventName];
    return "";
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	cocos2d::JniMethodInfo pInfo;
	if(cocos2d::JniHelper::getStaticMethodInfo(pInfo, "com/ms/comm/JniFuncUtils", "doFuncByEvent", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;")){
		jstring ret = (jstring)pInfo.env->CallStaticObjectMethod(pInfo.classID, pInfo.methodID, pInfo.env->NewStringUTF(funcName.c_str()),
			pInfo.env->NewStringUTF(arg_str.c_str()), pInfo.env->NewStringUTF(event_name.c_str()));
		return cocos2d::JniHelper::jstring2string(ret);
	}
	return "";
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	dispatchCommFuncEvent(event_name, "test:result,ret_arg1,ret_arg2"); //test
	return "";
#else
	return "undefined device";
#endif
	printf("doCommFuncByEvent Error Func not exist %s", funcName.c_str());
	return "doCommFuncByEvent Error Func not exist";
}

int lshift(int value, int n)
{
	int chgValue = value << n;
	return chgValue;
}

int rshift(int value, int n)
{
	int chgValue = value >> n;
	return chgValue;
}

int compressStr(string& src, ByteBuffer* dest, int& len_compress)
{
	unsigned long lenS = src.length();
	unsigned long lenD = compressBound(lenS);
	unsigned char* bufferS = (unsigned char*)(src.c_str());
	unsigned char* bufferD = new unsigned char[lenD];
	int ret = compress(bufferD,&lenD,bufferS,lenS);
	if(ret != Z_OK)
	{
		CCLOG("compressStr fail");
		return -1;
	}
	len_compress = (int)lenD;
	dest->Write(bufferD, lenD);
	
	/*unsigned char* un_buffer_tmp = new unsigned char[len_src];
	unsigned long  destlen;
	int unret = uncompress(un_buffer_tmp, &destlen, dest->read(len_compress), len_dest);
	un_buffer_tmp[len_src] = '\0';
	if(unret == Z_OK)
	{
		CCLOG("uncompressStr succ %s", (char*)un_buffer_tmp);
	}*/
	return 0;
}

int uncompressStr(ByteBuffer* src, ByteBuffer* dest, int len_compress, int len_src)
{
	unsigned char* un_bufferD = new unsigned char[len_src];
	unsigned long lenD;
	int ret = uncompress(un_bufferD, &lenD, src->read(len_compress), len_src);
	if( ret != Z_OK )
	{
		CCLOG("uncompress fail");
		return -1;
	}
	un_bufferD[len_src] = '\0';
	dest->Write(un_bufferD, lenD);
	return 0;
}


int b64CompressStr(const string& src, string& dest)
{
	uLong lenS = src.length();
	uLong lenD = compressBound(lenS);
	unsigned char* buf = (unsigned char*)malloc(sizeof(unsigned char) * lenD);
	if(!buf) {
		return -1;
	}
	memset(buf, '\0', lenD);
	if(compress(buf, &lenD, (unsigned char*)src.c_str(), lenS) != Z_OK) {
		free(buf);
		return -1;
	}
	char* b64_buff = NULL;
	unsigned long b64len = base64Encode(buf, lenD, &b64_buff);
	dest.assign(b64_buff, b64len);
	free(buf);

	return 0;
}

int unb64UncompressStr(const string& src, const unsigned long srcLen, string& dest)
{
	unsigned char* buff = NULL;
	unsigned long lenS = base64Decode((unsigned char*)src.c_str(), src.length(), &buff);
    unsigned char* buf = (unsigned char*)malloc(sizeof(unsigned char) * (srcLen+1));
    if(!buf) {
        return -1;
    }
	memset(buf, '\0', srcLen+1);
    unsigned long lenD = srcLen;
    int ret = uncompress(buf, &lenD, buff, lenS);
    if(ret != Z_OK) {
        free(buf);
        return -1;
    }
	
    dest.assign((char*)buf, lenD);
    free(buf);
    
    return 0;
}