#include "cocos2d.h"
#include "FuncHelper.h"
#include "com_ms.comm.JniFuncUtils.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "../platform/android/jni/JniHelper.h"
#endif

using namespace cocos2d;

void Java_com_ms_comm_JniFuncUtils_JniFuncCallBack
  (JNIEnv * env, jobject, jstring event_name, jstring result) {
      string _event_name = cocos2d::JniHelper::jstring2string(event_name);
      string _result = cocos2d::JniHelper::jstring2string(result);
      CCLOG("JniFUncCallbBack...%s,%s",_event_name.c_str(),_result.c_str());
      dispatchCommFuncEvent(_event_name, _result);
}
