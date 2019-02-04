#ifndef __CC_TCPSOCKET_H__
#define __CC_TCPSOCKET_H__

#include <functional>
#include "ByteBuffer.h"
#ifdef WIN32
	#include <windows.h>
	#include <WinSock.h>
	#include <process.h>
	#pragma comment( lib, "ws2_32.lib" )
	#pragma comment( lib, "libzlib.lib" )
#else
	#include <sys/socket.h>
	#include <fcntl.h>
	#include <errno.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <pthread.h>

	#define SOCKET int
	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
#endif

#define _MAX_MSGSIZE 16 * 1024
#define BLOCKSECONDS	30
#define INBUFSIZE	(64*1024)
#define OUTBUFSIZE	(8*1024)
#define FLUSH_RETRY (5)

enum ECheckState{
	ecsOK = 0,
	ecsAgain,
	ecsIgnore,
	ecsError,
};

class  TCPSocket
{
public:
	TCPSocket(void);
	bool	Create(const char* pszServerIP, int nServerPort, int tagid, int nBlockSec = BLOCKSECONDS, bool bKeepAlive = false);
	bool	SendMsg(void* pBuf, int nSize);
	ECheckState	ReceiveMsg(void* pBuf, int& nSize);
	bool	Flush(void);
	bool	Check(void);
	void	Destroy(void);
	SOCKET	GetSocket(void) const { return m_sockClient; }
	
	int		getTagID(){ return m_tag; }
private:
	bool	recvFromSock(void);
	bool    hasError();
	void    closeSocket();
	ECheckState	CheckMsg(void* buf, int& nSize);
	SOCKET	m_sockClient;

	char	m_bufOutput[OUTBUFSIZE];
	int		m_nOutbufLen;

	char	m_bufInput[INBUFSIZE];
	int		m_nInbufLen;
	int		m_nInbufStart;
	int		m_tag;
};

typedef bool(*ProAllFunc)(int,int,ByteBuffer&);
typedef void(*ProFunc)(int,ByteBuffer&);
typedef void(*sckConnectFunc)(int,bool);
typedef void(*sckDisconnectFunc)(int);

#define SCT_CALLBACK_1(func, _Object) std::bind(&func,_Object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
#define SCT_CALLBACK_2(func, _Object) std::bind(&func,_Object, std::placeholders::_1, std::placeholders::_2)
#define SCT_CALLBACK_3(func, _Object) std::bind(&func,_Object, std::placeholders::_1)

#define CREATE_TCPSOCKETMGR()	new TCPSocketManager()
#define CMSG_PING		0x01
#define PINGINTERVAL_TIMER	1

struct SocketAddress
{
	std::string pserverip;
	int nPort;
	int nTag;
	SocketAddress()
	{
		pserverip = "";
		nPort = 0;
		nTag = 0;
	}
};

struct socketConnectData
{
	int socket_tag;
	bool b_connect_succ;
};

struct PacketData
{
	int _sockettag;
	ByteBuffer buffer;
};

	class Delegate
    {
    public:
        virtual ~Delegate() {}
    };

class TCPSocketManager
{
public:

	enum class ErrorCode
    {
        TIME_OUT,
        CONNECTION_FAILURE,
        UNKNOWN,
    };

	TCPSocketManager();
	~TCPSocketManager();

	void setDelegate(Delegate* _delegate){m_delegate=_delegate;}
	void createSocket(const char* pszServerIP,
							int nServerPort,
							int _tag,
							int nBlockSec = BLOCKSECONDS,
							bool bKeepAlive = false);
	void destroySocket(int _tag);
	bool	connect(SocketAddress *pAddress);
	bool	addSocket(TCPSocket *pSocket);
	bool	removeSocket(int _tag);
	void	disconnect(int _tag);
	TCPSocket	*GetSocket(int _tag);
	bool	SendPacket(int _tag, ByteBuffer *buffer);

	void	update();

	void	updateMsg();

	uint64	getClientStartTime();

	static TCPSocketManager * getSingleton(){
		assert(mSingleton); return mSingleton;}

#ifdef _WINDOWS
	static void socket_thread(void *mgr);
	CRITICAL_SECTION mCs;
#else
	static void *socket_thread(void *mgr);
	mutable pthread_mutex_t m_mutex;
#endif
protected:
	void	SendPingPacket(ByteBuffer &packet);
	
private:	
	ProAllFunc _pProcess;
	sckConnectFunc _pOnConnect;
	sckDisconnectFunc _OnDisconnect;
	std::list<TCPSocket*> m_lstSocket;
	std::map<uint16, ProFunc> _mapProcess;
	static TCPSocketManager * mSingleton;
	time_t m_sendping_last_time;
	std::list<SocketAddress> m_lstAddress;
	Delegate* m_delegate;
	uint64	m_startTime;
};

#define sSocketMgr	TCPSocketManager::getSingleton()


#endif //__CC_TCPSOCKET_H__
