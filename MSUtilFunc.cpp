
#include "MSUtilFunc.h"
#include "cocos2d.h"

USING_NS_CC;

namespace msUtilFunc {

static void _log_skill(int color, const char* format, va_list args)
{
	// �������ֱ��ʹ��cocos2d::log������������
#if CC_TARGET_PLATFORM ==  CC_PLATFORM_WIN32
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
	SetConsoleTextAttribute(hConsole, color);
	log_valist(format, args);
	SetConsoleTextAttribute(hConsole, 7);
#else
	log_valist(format, args);
#endif
}

void log_skill(int color, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	_log_skill(color, format, args);
	va_end(args);
}

}