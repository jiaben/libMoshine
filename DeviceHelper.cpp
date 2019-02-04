#include "DeviceHelper.h"
#include "cocos2d.h"
#include "CodeCvs.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#import  <CoreTelephony/CTTelephonyNetworkInfo.h>
#import<CoreTelephony/CTCarrier.h>
#import "IdentifierAddition.h"
#include <sys/types.h>
#include <sys/sysctl.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#import "math.h"
#include <mach/mach_host.h>
#import <AdSupport/AdSupport.h>
#import <SystemConfiguration/SCNetworkConfiguration.h>
#import <mach/host_info.h>
#import <mach/mach_host.h>
#import <mach/task_info.h>
#import <mach/task.h>
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <WinSock2.h>
#include <Iphlpapi.h>
#pragma comment(lib,"Iphlpapi.lib")
#endif

string getDeviceUniqueIdentifier()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    if ( [[UIDevice currentDevice].systemVersion floatValue] < 6.0)
    {
        //    	NSString *strId = [[UIDevice currentDevice] uniqueIdentifier]; // A GUID like string
        NSString *strId = [[UIDevice currentDevice] uniqueDeviceIdentifier];
        return [strId UTF8String];
    }
    else{
        NSUUID* idForVendor = [[UIDevice currentDevice] identifierForVendor];
        return [[idForVendor UUIDString] UTF8String];
    }
#endif
    return "undefined device";
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
NSString* getIPAddress() {
    NSString *address = @"unknown";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    
    if(getifaddrs(&interfaces) == 0) {
        temp_addr = interfaces;
        while (temp_addr != NULL) {
            if (temp_addr->ifa_addr->sa_family == AF_INET) {
                NSString *name = [NSString stringWithUTF8String:temp_addr->ifa_name];
                // en0 is the local wifi ip address and lo0 is 127.0.0.1
                if (![name isEqualToString:@"en0"]
                    && ![name isEqualToString:@"lo0"]) {
                    address = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr)];
                    break;
                }
            }
            
            temp_addr = temp_addr->ifa_next;
        }
    }
    free(interfaces);
    
    return address;
}
#endif


#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
bool getWinMacAddress(string& macstring)
{
	bool ret = false;
	ULONG ipInfoLen = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO adapterInfo = (IP_ADAPTER_INFO *)malloc(ipInfoLen);
	if (adapterInfo == NULL)
	{
		return false;
	}

	if (GetAdaptersInfo(adapterInfo, &ipInfoLen) == ERROR_BUFFER_OVERFLOW)
	{
		free(adapterInfo);
		adapterInfo = (IP_ADAPTER_INFO *)malloc(ipInfoLen);
		if (adapterInfo == NULL)
		{
			return false;
		}
	}

	if (GetAdaptersInfo(adapterInfo, &ipInfoLen) == NO_ERROR)
	{
		for (PIP_ADAPTER_INFO pAdapter = adapterInfo; pAdapter != NULL; pAdapter = pAdapter->Next)
		{
			if (pAdapter->Type != MIB_IF_TYPE_ETHERNET)
			{
				continue;
			}

			if (pAdapter->AddressLength != 6)
			{
				continue;
			}

			char buf32[32];
			sprintf(buf32, "%02X-%02X-%02X-%02X-%02X-%02X",
				int(pAdapter->Address[0]),
				int(pAdapter->Address[1]),
				int(pAdapter->Address[2]),
				int(pAdapter->Address[3]),
				int(pAdapter->Address[4]),
				int(pAdapter->Address[5]));
			macstring = buf32;
			ret = true;
			break;
		}
	}

	free(adapterInfo);
	return ret;
}
#endif



#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
//判断设备机型
#define iPhone6 ([UIScreen instancesRespondToSelector:@selector(currentMode)] ? (CGSizeEqualToSize(CGSizeMake(750, 1334), [[UIScreen mainScreen] currentMode].size) || CGSizeEqualToSize(CGSizeMake(640, 1136), [[UIScreen mainScreen] currentMode].size)) : NO)
#define iPhone6PlusAmplification ([UIScreen instancesRespondToSelector:@selector(currentMode)] ? CGSizeEqualToSize(CGSizeMake(1125,2001) , [[UIScreen mainScreen] currentMode].size) : NO)//iPhone 6 plus有两种显示模式，标准模式分辨率为1242x2208，放大模式分辨率为1125x2001
#define iPhone6Plus ([UIScreen instancesRespondToSelector:@selector(currentMode)] ? CGSizeEqualToSize(CGSizeMake(1242,2208) , [[UIScreen mainScreen] currentMode].size) : NO)
#define iPhone5 ([UIScreen instancesRespondToSelector:@selector(currentMode)] ? CGSizeEqualToSize(CGSizeMake(640,1136), [[UIScreen mainScreen] currentMode].size) : NO)
#define iPhone4 ([UIScreen instancesRespondToSelector:@selector(currentMode)] ? CGSizeEqualToSize(CGSizeMake(320,480), [[UIScreen mainScreen] currentMode].size) : NO)
#define iPhone4s ([UIScreen instancesRespondToSelector:@selector(currentMode)] ? CGSizeEqualToSize(CGSizeMake(640,960), [[UIScreen mainScreen] currentMode].size) : NO)
string GetNetWorkType()
{
    string strNetworkType = "";
    
    //创建零地址，0.0.0.0的地址表示查询本机的网络连接状态
    struct sockaddr_storage zeroAddress;
    
    bzero(&zeroAddress, sizeof(zeroAddress));
    zeroAddress.ss_len = sizeof(zeroAddress);
    zeroAddress.ss_family = AF_INET;
    
    // Recover reachability flags
    SCNetworkReachabilityRef defaultRouteReachability = SCNetworkReachabilityCreateWithAddress(NULL, (struct sockaddr *)&zeroAddress);
    SCNetworkReachabilityFlags flags;
    
    //获得连接的标志
    BOOL didRetrieveFlags = SCNetworkReachabilityGetFlags(defaultRouteReachability, &flags);
    CFRelease(defaultRouteReachability);
    
    //如果不能获取连接标志，则不能连接网络，直接返回
    if (!didRetrieveFlags)
    {
        return strNetworkType;
    }
    
    
    if ((flags & kSCNetworkReachabilityFlagsConnectionRequired) == 0)
    {
        // if target host is reachable and no connection is required
        // then we'll assume (for now) that your on Wi-Fi
        strNetworkType = "WIFI";
    }
    
    if (
        ((flags & kSCNetworkReachabilityFlagsConnectionOnDemand ) != 0) ||
        (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic) != 0
        )
    {
        // ... and the connection is on-demand (or on-traffic) if the
        // calling application is using the CFSocketStream or higher APIs
        if ((flags & kSCNetworkReachabilityFlagsInterventionRequired) == 0)
        {
            // ... and no [user] intervention is needed
            strNetworkType = "WIFI";
        }
    }
    
    if ((flags & kSCNetworkReachabilityFlagsIsWWAN) == kSCNetworkReachabilityFlagsIsWWAN)
    {
        if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 7.0)
        {
            CTTelephonyNetworkInfo * info = [[CTTelephonyNetworkInfo alloc] init];
            NSString *currentRadioAccessTechnology = info.currentRadioAccessTechnology;
            
            if (currentRadioAccessTechnology)
            {
                if ([currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyLTE])
                {
                    strNetworkType =  "4G";
                }
                else if ([currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyEdge] || [currentRadioAccessTechnology isEqualToString:CTRadioAccessTechnologyGPRS])
                {
                    strNetworkType =  "2G";
                }
                else
                {
                    strNetworkType =  "3G";
                }
            }
        }
        else
        {
            if((flags & kSCNetworkReachabilityFlagsReachable) == kSCNetworkReachabilityFlagsReachable)
            {
                if ((flags & kSCNetworkReachabilityFlagsTransientConnection) == kSCNetworkReachabilityFlagsTransientConnection)
                {
                    if((flags & kSCNetworkReachabilityFlagsConnectionRequired) == kSCNetworkReachabilityFlagsConnectionRequired)
                    {
                        strNetworkType = "2G";
                    }
                    else
                    {
                        strNetworkType = "3G";
                    }
                }
            }
        }
    }
    
    
    if (strNetworkType == "") {
        strNetworkType = "WWAN";
    }
    
    NSLog( @"GetNetWorkType() strNetworkType :  %s", strNetworkType.c_str());
    
    return strNetworkType;
}


NSString* getCPUType(int type)
{
    NSMutableString *cpu = [[NSMutableString alloc] init];
    // values for cputype and cpusubtype defined in mach/machine.h
    if (type == CPU_TYPE_X86)
    {
        [cpu appendString:@"CPU_TYPE_X86"];
    }
    else if(type == CPU_TYPE_ANY)
    {
        [cpu appendString:@"CPU_TYPE_ANY"];
    }
    else if(type ==CPU_TYPE_VAX)
    {
        [cpu appendString:@"CPU_TYPE_VAX"];
    }
    else if(type == CPU_TYPE_MC680x0)
    {
        [cpu appendString:@"CPU_TYPE_MC680x0"];
    }
    else if(type == CPU_TYPE_I386)
    {
        [cpu appendString:@"CPU_TYPE_I386"];
    }
    else if(type == CPU_TYPE_X86_64)
    {
        [cpu appendString:@"CPU_TYPE_X86_64"];
    }
    
    else if(type ==CPU_TYPE_MC98000)
    {
        [cpu appendString:@"CPU_TYPE_MC98000"];
    }
    else if(type == CPU_TYPE_ARM)
    {
        [cpu appendString:@"CPU_TYPE_ARM"];
    }
    else if(type == CPU_TYPE_ARM64)
    {
        [cpu appendString:@"CPU_TYPE_ARM64"];
    }
    else if(type == CPU_TYPE_MC88000)
    {
        [cpu appendString:@"CPU_TYPE_MC88000"];
    }
    else if(type == CPU_TYPE_SPARC)
    {
        [cpu appendString:@"CPU_TYPE_SPARC"];
    }
    else if(type == CPU_TYPE_I860)
    {
        [cpu appendString:@"CPU_TYPE_I860"];
    }
    else if (type ==CPU_TYPE_POWERPC)
    {
        [cpu appendString:@"CPU_TYPE_POWERPC"];
    }
    return [cpu autorelease];
    
}

NSString *const Device_Simulator = @"Simulator";
NSString *const Device_iPod1 = @"iPod1";
NSString *const Device_iPod2 = @"iPod2";
NSString *const Device_iPod3 = @"iPod3";
NSString *const Device_iPod4 = @"iPod4";
NSString *const Device_iPod5 = @"iPod5";
NSString *const Device_iPad2 = @"iPad2";
NSString *const Device_iPad3 = @"iPad3";
NSString *const Device_iPad4 = @"iPad4";
NSString *const Device_iPhone4 = @"iPhone 4";
NSString *const Device_iPhone4S = @"iPhone 4S";
NSString *const Device_iPhone5 = @"iPhone 5";
NSString *const Device_iPhone5S = @"iPhone 5S";
NSString *const Device_iPhone5C = @"iPhone 5C";
NSString *const Device_iPadMini1 = @"iPad Mini 1";
NSString *const Device_iPadMini2 = @"iPad Mini 2";
NSString *const Device_iPadMini3 = @"iPad Mini 3";
NSString *const Device_iPadAir1 = @"iPad Air 1";
NSString *const Device_iPadAir2 = @"iPad Mini 3";
NSString *const Device_iPhone6 = @"iPhone 6";
NSString *const Device_iPhone6plus = @"iPhone 6 Plus";
NSString *const Device_iPhone6S = @"iPhone 6S";
NSString *const Device_iPhone6Splus = @"iPhone 6S Plus";
NSString *const Device_Unrecognized = @"?unrecognized?";

NSString* getDeviceName()
{
    

    size_t size;
    int nR = sysctlbyname("hw.machine", NULL, &size, NULL, 0);
    char *machine = (char *)malloc(size);
    nR = sysctlbyname("hw.machine", machine, &size, NULL, 0);
    NSString *platform = [NSString stringWithCString:machine encoding:NSUTF8StringEncoding];
    free(machine);
    static NSDictionary* deviceNamesByCode = nil;
    
    if (!deviceNamesByCode) {
        
        deviceNamesByCode = @{
                              @"i386"      : Device_Simulator,
                              @"x86_64"    : Device_Simulator,
                              @"iPod1,1"   : Device_iPod1,
                              @"iPod2,1"   : Device_iPod2,
                              @"iPod3,1"   : Device_iPod3,
                              @"iPod4,1"   : Device_iPod4,
                              @"iPod5,1"   : Device_iPod5,
                              @"iPad2,1"   : Device_iPad2,
                              @"iPad2,2"   : Device_iPad2,
                              @"iPad2,3"   : Device_iPad2,
                              @"iPad2,4"   : Device_iPad2,
                              @"iPad2,5"   : Device_iPadMini1,
                              @"iPad2,6"   : Device_iPadMini1,
                              @"iPad2,7"   : Device_iPadMini1,
                              @"iPhone3,1" : Device_iPhone4,
                              @"iPhone3,2" : Device_iPhone4,
                              @"iPhone3,3" : Device_iPhone4,
                              @"iPhone4,1" : Device_iPhone4S,
                              @"iPhone5,1" : Device_iPhone5,
                              @"iPhone5,2" : Device_iPhone5,
                              @"iPhone5,3" : Device_iPhone5C,
                              @"iPhone5,4" : Device_iPhone5C,
                              @"iPad3,1"   : Device_iPad3,
                              @"iPad3,2"   : Device_iPad3,
                              @"iPad3,3"   : Device_iPad3,
                              @"iPad3,4"   : Device_iPad4,
                              @"iPad3,5"   : Device_iPad4,
                              @"iPad3,6"   : Device_iPad4,
                              @"iPhone6,1" : Device_iPhone5S,
                              @"iPhone6,2" : Device_iPhone5S,
                              @"iPad4,1"   : Device_iPadAir1,
                              @"iPad4,2"   : Device_iPadAir2,
                              @"iPad4,4"   : Device_iPadMini2,
                              @"iPad4,5"   : Device_iPadMini2,
                              @"iPad4,6"   : Device_iPadMini2,
                              @"iPad4,7"   : Device_iPadMini3,
                              @"iPad4,8"   : Device_iPadMini3,
                              @"iPad4,9"   : Device_iPadMini3,
                              @"iPhone7,1" : Device_iPhone6plus,
                              @"iPhone7,2" : Device_iPhone6,
                              @"iPhone8,1" : Device_iPhone6S,
                              @"iPhone8,2" : Device_iPhone6Splus
                              };
    }
    
    NSString* deviceName = [deviceNamesByCode objectForKey:platform];
    if(deviceName){
        return deviceName;
    }
    return Device_Unrecognized;
}


NSUInteger getTotalMemoryBytes()
{
    size_t size = sizeof(int);
    int results;
    int mib[2] = {CTL_HW, HW_PHYSMEM};
    sysctl(mib, 2, &results, &size, NULL, 0);
    return (NSUInteger) results/1024/1024;
}

string getDeviceInfo() {
    NSString * deviceInfo = [NSString string];
    //设备id
    NSString * deviceId = [[UIDevice currentDevice] uniqueGlobalDeviceIdentifier];
    //设备类型
    NSString * model = [[UIDevice currentDevice] model];
    //系统版本号
    NSString * systemVersion = [[UIDevice currentDevice] systemVersion];
    //获取当前网络的类型
    CTTelephonyNetworkInfo  *networkInfo = [[ CTTelephonyNetworkInfo   alloc ] init ];
   // NSString * network = networkInfo.currentRadioAccessTechnology;
    NSString * network = [NSString stringWithUTF8String:GetNetWorkType().c_str()];
    //获取运行商的名称
    CTCarrier *carrier = [networkInfo subscriberCellularProvider];
    NSString *mCarrier = [NSString stringWithFormat:@"%@",[carrier carrierName]];
    
    //分辨率
    CGRect rect = [[UIScreen mainScreen] bounds];
    CGFloat scale = [[UIScreen mainScreen] scale];
    CGFloat width = rect.size.width * scale;
    CGFloat height = rect.size.height * scale;
    

    //屏幕尺寸对角线长度
//    float screenInch = sqrt(width*width+height*height);
//    NSString* screenInch = [NSString stringWithFormat:@"%f",screenInch];
    //像素密度 每英寸所拥有的像素数目
    
//    double ppi = sqrt(width*width+height*height)/4.7;
//    NSString* screenppi = [NSString stringWithFormat:@"%f",ppi];;
    //内存
    host_basic_info_data_t hostInfo;
    mach_msg_type_number_t infoCount;
    infoCount = HOST_BASIC_INFO_COUNT;
    host_info(mach_host_self(),HOST_BASIC_INFO_COUNT,(host_info_t)&hostInfo,&infoCount);
    NSString* memSize = [NSString stringWithFormat:@"%d",hostInfo.memory_size/1024/1024];
    NSInteger mem_total = getTotalMemoryBytes();
   //unsigned long long Memory = [NSProcessInfo processInfo].physicalMemory;
    
    //cpu类型
    NSString *cpuType = getCPUType(hostInfo.cpu_type);
    //cpu 主频
    
//    int mib[2];
//    int result;
//    mib[0] = CTL_HW;
//    mib[1] = HW_CPU_FREQ;
//    size_t length = sizeof(result);
//    if (sysctl(mib, 2, &result, &length, NULL, 0) < 0)
//    {
//        perror("getting cpu frequency");
//    }
    unsigned int cpufrequency;
    size_t len_cpufrequency =sizeof(cpufrequency);
    sysctlbyname("hw.cpufrequency", &cpufrequency, &len_cpufrequency, NULL, 0);
   //cpu 核数
    
    unsigned int ncpu;
    size_t len =sizeof(ncpu);
    sysctlbyname("hw.ncpu", &ncpu, &len, NULL, 0);
    NSString* cpuCore = [NSString stringWithFormat:@"%d",ncpu];
   
    NSString *adId = [[[ASIdentifierManager sharedManager] advertisingIdentifier] UUIDString];
    
    
    deviceInfo = [deviceInfo stringByAppendingFormat:@"deviceid=%@,",deviceId];
    deviceInfo = [deviceInfo stringByAppendingFormat:@"model=%@,",model];
    deviceInfo = [deviceInfo stringByAppendingFormat:@"service=%@,",mCarrier];
    deviceInfo = [deviceInfo stringByAppendingFormat:@"network=%@,",network];
    deviceInfo = [deviceInfo stringByAppendingFormat:@"screenwidth=%d,",(int)width];
    deviceInfo = [deviceInfo stringByAppendingFormat:@"screenheight=%d,",(int)height];
    //deviceInfo = [deviceInfo stringByAppendingFormat:@"screenppi=%@,",screenppi];
    //deviceInfo = [deviceInfo stringByAppendingFormat:@"screensize=%@,",screensize];
    deviceInfo = [deviceInfo stringByAppendingFormat:@"cputype=%@,",cpuType];
    deviceInfo = [deviceInfo stringByAppendingFormat:@"cpuhz=%dGhz,",cpufrequency];
    deviceInfo = [deviceInfo stringByAppendingFormat:@"cpucore=%@,",cpuCore];
    deviceInfo = [deviceInfo stringByAppendingFormat:@"ramsize=%@MB,",memSize];
    deviceInfo = [deviceInfo stringByAppendingFormat:@"systemVersion=%@,",systemVersion];
    deviceInfo = [deviceInfo stringByAppendingFormat:@"idfa=%@",adId];
    return [deviceInfo UTF8String];
}
#endif