/////////////////////////////////////////////////////////////////////////////
//===========================================================================
#define _WINSOCKAPI_
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define NOMINMAX



/////////////////////////////////////////////////////////////////////////////
//===========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//===========================================================================
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

//===========================================================================
#include <crtdbg.h>
#include <Windows.h>

//===========================================================================
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <mstcpip.h>



/////////////////////////////////////////////////////////////////////////////
//===========================================================================
// C:\Program Files(x86)\Windows Kits\10\Include   \10.0.19041.0\um
// C:\Program Files(x86)\Windows Kits\10\Lib       \10.0.19041.0\um\x64
#pragma comment(lib, "ws2_32.lib")



/////////////////////////////////////////////////////////////////////////////
//===========================================================================
#define MULTICAST_TEST

//===========================================================================
#define LOCAL_IP  "192.168.1.14"
//#define LOCAL_IP  "127.0.0.1"

//===========================================================================
#define GROUP_IP1 "236.1.10.11"
#define GROUP_IP2 "236.1.10.12"
#define RX1_BIND_PORT 2000
#define RX2_BIND_PORT 2000
#define TX1_BIND_PORT 2001
#define TX2_BIND_PORT 2002

#define RX3_BIND_PORT 2000
#define TX3_BIND_PORT 2003

//===========================================================================
// 통신안됨
//#define BUF_SIZE (1024*64-28) // 65508

//===========================================================================
// 통신됨 
#define BUF_SIZE (1024*64-29) // = 65507
/// 65507 + 28 = 65535
//          20 = IP  header size
//           8 = UDP header size

//===========================================================================
// IEEE 802.3 Ethernet Frame
// PHY head    = 8
// ETH head    = 6 + 6 + 2
// ETH payload = 1500
//               IP 20 / TCP 20
//               IP 20 / UDP  8
// ETH FCS     = 4
//             = 1518
//


/////////////////////////////////////////////////////////////////////////////
//===========================================================================
void test_rx1(void)
{
	int rv;

	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	std::string _local_ip = LOCAL_IP;
	std::string _group_ip = GROUP_IP1;
#else
	std::string _local_ip = "127.0.0.1";
	std::string _group_ip = "127.0.0.1";
#endif

	//-----------------------------------------------------------------------
	SOCKET _socket;


	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


	//-----------------------------------------------------------------------
	int _reuse;


	_reuse = 1;
	rv = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&_reuse, sizeof(_reuse));
	printf("[RX1] setsockopt(SO_REUSEADDR)=%d\n", rv);


	//-----------------------------------------------------------------------
	Sleep(1000); // unicast 소켓이 먼저 bind()를 호출해야 unicast 패킷이 그 쪽 소켓으로 수신
	//-----------------------------------------------------------------------


	//-----------------------------------------------------------------------
	struct sockaddr_in _bind_address;


	_bind_address.sin_family = AF_INET;
	_bind_address.sin_addr.s_addr = inet_addr(_local_ip.c_str());
	_bind_address.sin_port = htons(RX1_BIND_PORT);

	rv = bind(_socket, (struct sockaddr*)&_bind_address, sizeof(_bind_address));
	printf("[RX1] bind()=%d\n", rv);


	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	struct ip_mreq _interface;


	memset(&_interface, 0, sizeof(_interface));
	_interface.imr_interface.s_addr = inet_addr(_local_ip.c_str());
	_interface.imr_multiaddr.s_addr = inet_addr(_group_ip.c_str());

	rv = setsockopt(_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char*>(&_interface), sizeof(_interface));
	printf("[RX1] setsockopt(IP_ADD_MEMBERSHIP)=%d\n", rv);
#endif

	//-----------------------------------------------------------------------
	while (1)
	{
		//-------------------------------------------------------------------
		struct sockaddr from;
		unsigned int    fromlen;

		char buf[BUF_SIZE];
		int  len;

		int flags = 0;


		fromlen = sizeof(from);
		len = sizeof(buf);

		rv = ::recvfrom(_socket, buf, len, flags, &from, reinterpret_cast<int*>(&fromlen));
		if (rv > 0)
		{
			printf("[RX1] rx = %d : %s \r\n", rv, buf);
		}


		//Sleep(1000);
	}


	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	rv = setsockopt(_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<char*>(&_interface), sizeof(_interface));
	printf("[RX1] setsockopt(IP_DROP_MEMBERSHIP)=%d\n", rv);
#endif

	//-----------------------------------------------------------------------
	closesocket(_socket);
}

void test_tx1(void)
{
	int rv;

	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	std::string _local_ip = LOCAL_IP;
	std::string _group_ip = GROUP_IP1;
#else
	std::string _local_ip = "127.0.0.1";
	std::string _group_ip = "127.0.0.1";
#endif


	//-----------------------------------------------------------------------
	SOCKET _socket;


	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


	//-----------------------------------------------------------------------
	int _reuse;


	_reuse = 1;
	rv = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&_reuse, sizeof(_reuse));
	printf("[TX1] setsockopt(SO_REUSEADDR)=%d\n", rv);


	//-----------------------------------------------------------------------
	struct sockaddr_in _bind_address;


	_bind_address.sin_family = AF_INET;
	_bind_address.sin_addr.s_addr = inet_addr(_local_ip.c_str());
	_bind_address.sin_port = htons(TX1_BIND_PORT);

	rv = bind(_socket, (struct sockaddr*)&_bind_address, sizeof(_bind_address));
	printf("[TX1] bind()=%d\n", rv);


	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	char _multicast_ttl;


	_multicast_ttl = 64;
	rv = ::setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<char*>(&_multicast_ttl), sizeof(_multicast_ttl));
	printf("[TX1] setsockopt(IP_MULTICAST_TTL)=%d\n", rv);


	//-----------------------------------------------------------------------
	char _multicast_loop;


	_multicast_loop = 1;
	rv = setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_LOOP, reinterpret_cast<char*>(&_multicast_loop), sizeof(_multicast_loop));
	printf("[TX1] setsockopt(IP_MULTICAST_LOOP)=%d\n", rv);


	//-----------------------------------------------------------------------
	struct in_addr _interface_address;


	memset(&_interface_address, 0, sizeof(_interface_address));
	_interface_address.s_addr = inet_addr(_local_ip.c_str());
	rv = setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<char*>(&_interface_address), sizeof(_interface_address));
	printf("[TX1] setsockopt(IP_MULTICAST_IF)=%d\n", rv);
#endif

	//-----------------------------------------------------------------------
	while (1)
	{
		//-------------------------------------------------------------------
		struct sockaddr_in to;
		unsigned int       tolen;

		char buf[BUF_SIZE];
		int  len;

		int  flags = 0;


		memset(&to, 0, sizeof(to));
		to.sin_family = AF_INET;
		to.sin_port = htons(RX1_BIND_PORT);
		to.sin_addr.s_addr = inet_addr(_group_ip.c_str());
		tolen = sizeof(to);

		len = sizeof(buf);
		strcpy(buf, "hello-TX1:236.1.10.11");


		rv = ::sendto(_socket, const_cast<char*>(buf), len, flags, reinterpret_cast<struct sockaddr*>(&to), tolen);
		if (rv > 0)
		{
			//printf("[TX1] tx = %d : %s \r\n", rv, buf);
		}


		Sleep(1000);
	}

	//-----------------------------------------------------------------------
	closesocket(_socket);
}



/////////////////////////////////////////////////////////////////////////////
//===========================================================================
void test_rx2(void)
{
	int rv;

	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	std::string _local_ip = LOCAL_IP;
	std::string _group_ip = GROUP_IP2;
#else
	std::string _local_ip = "127.0.0.1";
	std::string _group_ip = "127.0.0.1";
#endif

	//-----------------------------------------------------------------------
	SOCKET _socket;


	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


	//-----------------------------------------------------------------------
	int _reuse;


	_reuse = 1;
	rv = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&_reuse, sizeof(_reuse));
	printf("[RX2] setsockopt(SO_REUSEADDR)=%d\n", rv);


	//-----------------------------------------------------------------------
	Sleep(2000); // unicast 소켓이 먼저 bind()를 호출해야 unicast 패킷이 그 쪽 소켓으로 수신
	//-----------------------------------------------------------------------


	//-----------------------------------------------------------------------
	struct sockaddr_in _bind_address;


	_bind_address.sin_family = AF_INET;
	_bind_address.sin_addr.s_addr = inet_addr(_local_ip.c_str());
	_bind_address.sin_port = htons(RX2_BIND_PORT);

	rv = bind(_socket, (struct sockaddr*)&_bind_address, sizeof(_bind_address));
	printf("[RX2] bind()=%d\n", rv);


	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	struct ip_mreq _interface;


	memset(&_interface, 0, sizeof(_interface));
	_interface.imr_interface.s_addr = inet_addr(_local_ip.c_str());
	_interface.imr_multiaddr.s_addr = inet_addr(_group_ip.c_str());

	rv = setsockopt(_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char*>(&_interface), sizeof(_interface));
	printf("[RX2] setsockopt(IP_ADD_MEMBERSHIP)=%d\n", rv);
#endif

	//-----------------------------------------------------------------------
	while (1)
	{
		//-------------------------------------------------------------------
		struct sockaddr from;
		unsigned int    fromlen;

		char buf[BUF_SIZE];
		int  len;

		int flags = 0;


		fromlen = sizeof(from);
		len = sizeof(buf);

		rv = ::recvfrom(_socket, buf, len, flags, &from, reinterpret_cast<int*>(&fromlen));
		if (rv > 0)
		{
			printf("[RX2] rx = %d : %s \r\n", rv, buf);
		}


		//Sleep(1000);
	}


	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	rv = setsockopt(_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<char*>(&_interface), sizeof(_interface));
	printf("[RX2] setsockopt(IP_DROP_MEMBERSHIP)=%d\n", rv);
#endif

	//-----------------------------------------------------------------------
	closesocket(_socket);
}

void test_tx2(void)
{
	int rv;

	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	std::string _local_ip = LOCAL_IP;
	std::string _group_ip = GROUP_IP2;
#else
	std::string _local_ip = "127.0.0.1";
	std::string _group_ip = "127.0.0.1";
#endif


	//-----------------------------------------------------------------------
	SOCKET _socket;


	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


	//-----------------------------------------------------------------------
	int _reuse;


	_reuse = 1;
	rv = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&_reuse, sizeof(_reuse));
	printf("[TX2] setsockopt(SO_REUSEADDR)=%d\n", rv);


	//-----------------------------------------------------------------------
	struct sockaddr_in _bind_address;


	_bind_address.sin_family = AF_INET;
	_bind_address.sin_addr.s_addr = inet_addr(_local_ip.c_str());
	_bind_address.sin_port = htons(TX2_BIND_PORT);

	rv = bind(_socket, (struct sockaddr*)&_bind_address, sizeof(_bind_address));
	printf("[TX2] bind()=%d\n", rv);


	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	char _multicast_ttl;


	_multicast_ttl = 64;
	rv = ::setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<char*>(&_multicast_ttl), sizeof(_multicast_ttl));
	printf("[TX2] setsockopt(IP_MULTICAST_TTL)=%d\n", rv);


	//-----------------------------------------------------------------------
	char _multicast_loop;


	_multicast_loop = 1;
	rv = setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_LOOP, reinterpret_cast<char*>(&_multicast_loop), sizeof(_multicast_loop));
	printf("[TX2] setsockopt(IP_MULTICAST_LOOP)=%d\n", rv);


	//-----------------------------------------------------------------------
	struct in_addr _interface_address;


	memset(&_interface_address, 0, sizeof(_interface_address));
	_interface_address.s_addr = inet_addr(_local_ip.c_str());
	rv = setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<char*>(&_interface_address), sizeof(_interface_address));
	printf("[TX2] setsockopt(IP_MULTICAST_IF)=%d\n", rv);
#endif

	//-----------------------------------------------------------------------
	while (1)
	{
		//-------------------------------------------------------------------
		struct sockaddr_in to;
		unsigned int       tolen;

		char buf[BUF_SIZE];
		int  len;

		int  flags = 0;


		memset(&to, 0, sizeof(to));
		to.sin_family = AF_INET;
		to.sin_port = htons(RX2_BIND_PORT);
		to.sin_addr.s_addr = inet_addr(_group_ip.c_str());
		tolen = sizeof(to);

		len = sizeof(buf);
		strcpy(buf, "hello-TX2:236.1.10.12");


		rv = ::sendto(_socket, const_cast<char*>(buf), len, flags, reinterpret_cast<struct sockaddr*>(&to), tolen);
		if (rv > 0)
		{
			//printf("[TX2] tx = %d : %s \r\n", rv, buf);
		}


		Sleep(1000);
	}

	//-----------------------------------------------------------------------
	closesocket(_socket);
}



/////////////////////////////////////////////////////////////////////////////
//===========================================================================
void test_rx3(void)
{
	int rv;

	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	std::string _local_ip = LOCAL_IP;
	std::string _group_ip = GROUP_IP2;
#else
	std::string _local_ip = "127.0.0.1";
	std::string _group_ip = "127.0.0.1";
#endif

	//-----------------------------------------------------------------------
	SOCKET _socket;


	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


	//-----------------------------------------------------------------------
	int _reuse;


	_reuse = 1;
	rv = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&_reuse, sizeof(_reuse));
	printf("[RX3] setsockopt(SO_REUSEADDR)=%d\n", rv);


	//-----------------------------------------------------------------------
	Sleep(0); // unicast 소켓이 먼저 bind()를 호출해야 unicast 패킷이 그 쪽 소켓으로 수신
	//-----------------------------------------------------------------------


	//-----------------------------------------------------------------------
	struct sockaddr_in _bind_address;


	_bind_address.sin_family = AF_INET;
	_bind_address.sin_addr.s_addr = inet_addr(_local_ip.c_str());
	_bind_address.sin_port = htons(RX3_BIND_PORT);

	rv = bind(_socket, (struct sockaddr*)&_bind_address, sizeof(_bind_address));
	printf("[RX3] bind()=%d\n", rv);


	//-----------------------------------------------------------------------
	while (1)
	{
		//-------------------------------------------------------------------
		struct sockaddr from;
		unsigned int    fromlen;

		char buf[BUF_SIZE];
		int  len;

		int flags = 0;


		fromlen = sizeof(from);
		len = sizeof(buf);

		rv = ::recvfrom(_socket, buf, len, flags, &from, reinterpret_cast<int*>(&fromlen));
		if (rv > 0)
		{
			printf("[RX3] rx = %d : %s \r\n", rv, buf);
		}


		//Sleep(1000);
	}

	//-----------------------------------------------------------------------
	closesocket(_socket);
}

void test_tx3(void)
{
	int rv;

	//-----------------------------------------------------------------------
#if defined(MULTICAST_TEST)
	std::string _local_ip = LOCAL_IP;
	std::string _group_ip = GROUP_IP2;
#else
	std::string _local_ip = "127.0.0.1";
	std::string _group_ip = "127.0.0.1";
#endif


	//-----------------------------------------------------------------------
	SOCKET _socket;


	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


	//-----------------------------------------------------------------------
	int _reuse;


	_reuse = 1;
	rv = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&_reuse, sizeof(_reuse));
	printf("[TX3] setsockopt(SO_REUSEADDR)=%d\n", rv);


	//-----------------------------------------------------------------------
	struct sockaddr_in _bind_address;


	_bind_address.sin_family = AF_INET;
	_bind_address.sin_addr.s_addr = inet_addr(_local_ip.c_str());
	_bind_address.sin_port = htons(TX3_BIND_PORT);

	rv = bind(_socket, (struct sockaddr*)&_bind_address, sizeof(_bind_address));
	printf("[TX3] bind()=%d\n", rv);


	//-----------------------------------------------------------------------
	while (1)
	{
		//-------------------------------------------------------------------
		struct sockaddr_in to;
		unsigned int       tolen;

		char buf[BUF_SIZE];
		int  len;

		int  flags = 0;


		memset(&to, 0, sizeof(to));
		to.sin_family = AF_INET;
		to.sin_port = htons(RX3_BIND_PORT);
		to.sin_addr.s_addr = inet_addr(_local_ip.c_str());
		tolen = sizeof(to);

		len = sizeof(buf);
		strcpy(buf, "hello-TX3");


		rv = ::sendto(_socket, const_cast<char*>(buf), len, flags, reinterpret_cast<struct sockaddr*>(&to), tolen);
		if (rv > 0)
		{
			//printf("[TX3] tx = %d : %s \r\n", rv, buf);
		}


		Sleep(1000);
	}

	//-----------------------------------------------------------------------
	closesocket(_socket);
}

DWORD WINAPI ThreadMain_1(LPVOID lpParam)
{
	test_tx1();
	return 0;
}

DWORD WINAPI ThreadMain_2(LPVOID lpParam)
{
	test_rx1();
	return 0;
}

DWORD WINAPI ThreadMain_3(LPVOID lpParam)
{
	test_tx2();
	return 0;
}

DWORD WINAPI ThreadMain_4(LPVOID lpParam)
{
	test_rx2();
	return 0;
}

DWORD WINAPI ThreadMain_5(LPVOID lpParam)
{
#if defined(MULTICAST_TEST)
	test_tx3();
#endif
	return 0;
}

DWORD WINAPI ThreadMain_6(LPVOID lpParam)
{
#if defined(MULTICAST_TEST)
	test_rx3();
#endif
	return 0;
}

int main()
{
	int rv;
	WSADATA wsadata;


	rv = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (rv != 0)
	{
		return -1;
	}

	DWORD  thread_id_1;
	HANDLE thread_1;

	DWORD  thread_id_2;
	HANDLE thread_2;

	DWORD  thread_id_3;
	HANDLE thread_3;

	DWORD  thread_id_4;
	HANDLE thread_4;


	DWORD  thread_id_5;
	HANDLE thread_5;

	DWORD  thread_id_6;
	HANDLE thread_6;


	thread_1 = CreateThread(NULL, 1024*1024, ThreadMain_1, NULL, 0, &thread_id_1);
	thread_2 = CreateThread(NULL, 1024*1024, ThreadMain_2, NULL, 0, &thread_id_2);
	thread_3 = CreateThread(NULL, 1024*1024, ThreadMain_3, NULL, 0, &thread_id_3);
	thread_4 = CreateThread(NULL, 1024*1024, ThreadMain_4, NULL, 0, &thread_id_4);
	thread_5 = CreateThread(NULL, 1024*1024, ThreadMain_5, NULL, 0, &thread_id_5);
	thread_6 = CreateThread(NULL, 1024*1024, ThreadMain_6, NULL, 0, &thread_id_6);

	
	WaitForSingleObject(thread_1, INFINITE);
	WaitForSingleObject(thread_2, INFINITE);
	WaitForSingleObject(thread_3, INFINITE);
	WaitForSingleObject(thread_4, INFINITE);
	WaitForSingleObject(thread_5, INFINITE);
	WaitForSingleObject(thread_6, INFINITE);

	CloseHandle(thread_1);
	CloseHandle(thread_2);
	CloseHandle(thread_3);
	CloseHandle(thread_4);
	CloseHandle(thread_5);
	CloseHandle(thread_6);

	WSACleanup();

	return 0;
}
