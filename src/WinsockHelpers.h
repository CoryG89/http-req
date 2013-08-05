#ifndef WINSOCK_HELPERS_H
#define WINSOCK_HELPERS_H
#define WIN32_LEAN_AND_MEAN
/******************************************************************************
 * WinsockHelpers.h
 *
 * Created By: Cory Gross, May 2, 2012
 * Description: Utility library specification containing general utility
 * functions meant for use in all kinds of projects.
 *****************************************************************************/
#include <stdexcept>
#include <iostream>
#include <sstream>

#include "WinSock2.h"
#include "WS2tcpip.h"
#include "Windows.h"

/************************************************/
/* Defined Networking Constants					*/
/************************************************/
#define WINSOCK_VER_2	2
#define REQ_WINSOCK_VER 2
#define	FTP_DATA_PORT	20
#define FTP_CMD_PORT	21
#define TELNET_PORT		23
#define DNS_PORT		53
#define HTTP_PORT		80
#define MAX_HEADER_SIZE 8190
#define	MAX_ERRMSG_SIZE	1024

/************************************************/
/* Winsock Macros								*/
/************************************************/
/** The following macro is useful if a program is built and ran without
	a way to cleanup the winsock api, this will cleanup on next run */
#define WSA_CLEAN_ONLY \
	WSACleanup(); \
	return 0;

/** Exception-safe Winsock Cleanup */
#define WSA_CLEANUP() \
	if (WSACleanup() != 0) \
		throw WinsockException("WSACleanup()");


/************************************************/
/* Winsock Exception Classes					*/
/************************************************/
class WinsockException	// all classes that inherit from WinsockException
{	private:			// call WSAGetLastError so require Windows
		static int errCode;  
		static char *errMsg;
		char *failure;

	public: 
		WinsockException()
		{
			errCode = WSAGetLastError();
			failure = NULL;

			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
                  FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, errCode,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (char *)errMsg, 1024, NULL);
		}
		WinsockException(char *failMsg)
		{
			WinsockException();
			failure = failMsg;
		}
		int errorCode() { return errCode; }
		const char *failMsg() { return failure; }
		static char *errorMsg() { return errMsg; }
};

char *WinsockException::errMsg = new char[1024];
int WinsockException::errCode = 0;

class HostResolveException : public WinsockException 
{ public: HostResolveException(): WinsockException() {}; 
		  HostResolveException(char *failMsg) : WinsockException(failMsg) {} 
};

class AddrInfoException : public WinsockException
{ public: AddrInfoException(): WinsockException() {}; 
		  AddrInfoException(char *failMsg) : WinsockException(failMsg) {}
};

class SocketException : public WinsockException 
{ public: SocketException() : WinsockException() {}; 
		  SocketException(char *failMsg) : WinsockException(failMsg) {}
};

class ConnectException : public SocketException
{ public: ConnectException() : SocketException() {} 
		  ConnectException(char *failMsg) : SocketException(failMsg) {}
};

class SendException : public SocketException
{ public: SendException() : SocketException() {} 
		  SendException(char *failMsg) : SocketException(failMsg) {}
};

class RecvException : public SocketException
{ public: RecvException() : SocketException() {} 
		  RecvException(char *failMsg) : SocketException(failMsg) {}
};

class BindException : public SocketException
{ public: BindException() : SocketException() {} 
		  BindException(char *failMsg) : SocketException(failMsg) {}
};


/************************************************/
/* STD Network Exception Classes				*/
/************************************************/

/** Any problems that do not cause an error code to be returned
	by WSAGetLastError in Winsock applications should use exceptions
	extended from NetworkException */

class NetworkException : public std::exception
{ public: NetworkException() : std::exception() {}
		  NetworkException(const char *whatMsg) : std::exception(whatMsg) {}
};

class ReqVersionException : public NetworkException
{ public: ReqVersionException() : NetworkException() {}
		  ReqVersionException(const char *whatMsg) : NetworkException(whatMsg) {}
};

/************************************************/
/* FUNCTIONS	       							*/
/************************************************/

/** Initialize WinSock */
WSADATA init_winsock(bool verbose = false)
{
	WSADATA wsaData;
	if (WSAStartup(REQ_WINSOCK_VER, &wsaData) != 0)
			throw WinsockException("WSAStartup()");
	if (LOBYTE(wsaData.wVersion) < REQ_WINSOCK_VER)
			throw ReqVersionException("Required Winsock version not installed");
	if (verbose)
		std::cout << "Log: Winsock initialization successful..." << std::endl
			 << "Log: Winsock version " 
			 << static_cast<int>(LOBYTE(wsaData.wVersion)) << " initialized..."
		     << std::endl;
	return wsaData;
}

/** Convenience function for creating a TCP socket */
SOCKET tcpSocket() {
	SOCKET tcpSocket;
	tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcpSocket == INVALID_SOCKET)
		throw SocketException();
	return tcpSocket;
}

/** Cast IP Addresses from addrinfo and sockaddr structures */
void *cast_in_addr(struct addrinfo *in)
{
	if (in->ai_family == AF_INET)
		return &(((struct sockaddr_in*)in->ai_addr)->sin_addr);
	else if (in->ai_family == AF_INET6)
		return &(((struct sockaddr_in6*)in->ai_addr)->sin6_addr);
	else
		return NULL;
}
void *cast_in_addr(struct sockaddr *in)
{
	if (in->sa_family == AF_INET)
		return &(((struct sockaddr_in*)in)->sin_addr);
	else if (in->sa_family == AF_INET6)
		return &(((struct sockaddr_in6*)in)->sin6_addr);
	else
		return NULL;
}

int ip4_addr(const char *hostname, struct in_addr &out)
{
	struct addrinfo hints, *res, *ptr;
	int result = 0;
	bool addrValid = false;

	/** Initialize hints structure */
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // ipv4 or ipv6 address family
	hints.ai_socktype = SOCK_STREAM;

	/** Function getaddrinfo returns non-zero if there's a problem */
	if (getaddrinfo(hostname, "http", &hints, &res) != 0)
		throw AddrInfoException("in4_string()");

	const int MAX_ATTEMPTS = 15;
	int i = 0;
	for (ptr = res; ptr != NULL; ptr->ai_next)
	{
		if (i++ <= MAX_ATTEMPTS)
		{
			if (ptr->ai_family == AF_INET)
			{
				out = (((struct sockaddr_in *)ptr->ai_addr)->sin_addr);
				addrValid = true;
				break;
			}
		}
		else
			break;
	}
	if (!addrValid)
		result = -1;
	freeaddrinfo(res);
	return result;
}

int ip6_addr(const char *hostname, in6_addr &out)
{
	struct addrinfo hints, *res, *ptr;
	int result = 0;
	bool addrValid = false;

	/** Initialize hints structure */
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // ipv4 or ipv6 address family
	hints.ai_socktype = SOCK_STREAM;

	/** Function getaddrinfo returns non-zero if there's a problem */
	if (getaddrinfo(hostname, "http", &hints, &res) != 0)
		throw AddrInfoException("in4_string()");

	const int MAX_ATTEMPTS = 15;
	int i = 0;
	for (ptr = res; ptr != NULL; ptr->ai_next)
	{
		if (i++ <= MAX_ATTEMPTS)
		{
			if (ptr->ai_family == AF_INET6)
			{
				out = (((struct sockaddr_in6 *)ptr->ai_addr)->sin6_addr);
				addrValid = true;
				break;
			}
		}
		else
			break;
	}
	if (!addrValid) { result = -1; };
	freeaddrinfo(res);
	return result;
}

/** Resolves a hostname using the getaddrinfo function. Allocates
	space for and returns a string representing either an IPv4
	or IPv6 address. */
char *ip_string(const char hostname[])
{
	char *buffer = new char[INET6_ADDRSTRLEN];
	struct addrinfo hints, *res, *ptr;
	void *addr = NULL;  // void pointer to be set to sin_addr or sin6_addr

	/** Initialize hints structure */
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // ipv4 or ipv6 address family
	hints.ai_socktype = SOCK_STREAM;

	/** Function getaddrinfo returns non-zero if there's a problem */
	if (getaddrinfo(hostname, "http", &hints, &res) != 0)
		throw WinsockException("getaddrinfo()");
	
	/** Loop through list of address structures till we find a valid IP */
	const int MAX_ATTEMPTS = 15;
	int i = 0;
	for (ptr = res; (ptr != NULL) && (i++ != MAX_ATTEMPTS); ptr = ptr->ai_next)
	{
		if (i++ <= MAX_ATTEMPTS)
		{
			if (ptr->ai_family == AF_INET)
			{		// Cast appropriately for ipv4
				addr = &(((struct sockaddr_in *)ptr->ai_addr)->sin_addr);
				inet_ntop(ptr->ai_family, (in_addr*) addr, buffer, INET_ADDRSTRLEN);
			}
			else if (ptr->ai_family == AF_INET6)
			{		// Cast appropraitely for ipv6
				addr = &(((struct sockaddr_in6 *)ptr->ai_addr)->sin6_addr);
				inet_ntop(ptr->ai_family, (in6_addr*) addr, buffer, INET6_ADDRSTRLEN);
			}
		}
	}

	freeaddrinfo(res);	// free list of address structures
	if (addr == NULL)
	{
		delete [] buffer;
		return NULL;
	}

	return buffer;
}


/** Resolves a hostname using the getaddrinfo function. Allocates
	space for and returns a string representing either an IPv4
	or IPv6 address. */
char *ip4_string(const char hostname[])
{
	char *buffer = new char[INET6_ADDRSTRLEN];
	struct addrinfo hints, *res, *ptr;
	in_addr *addr = NULL;

	/** Initialize hints structure */
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // ipv4 or ipv6 address family
	hints.ai_socktype = SOCK_STREAM;

	/** Function getaddrinfo returns non-zero if there's a problem */
	if (getaddrinfo(hostname, "http", &hints, &res) != 0)
		throw AddrInfoException("in4_string()");

	const int MAX_ATTEMPTS = 15;
	int i = 0;
	for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
	{
		if (i++ <= MAX_ATTEMPTS)
		{
			if (ptr->ai_family == AF_INET)
			{
				addr = &(((struct sockaddr_in *)ptr->ai_addr)->sin_addr);
				inet_ntop(ptr->ai_family, addr, buffer, INET6_ADDRSTRLEN);
				break;
			}
		}
	}

	freeaddrinfo(res);	// free list of address structures
	if (addr == NULL)
	{
		delete [] buffer;
		return NULL;
	}

	return buffer;
}

/** Resolves a hostname using the getaddrinfo function. Allocates
	space for and returns a string representing either an IPv4
	or IPv6 address. */
char *ip6_string(const char hostname[])
{
	char *buffer = new char[INET6_ADDRSTRLEN];
	struct addrinfo hints, *res, *ptr;
	in6_addr *addr = NULL;

	/** Initialize hints structure */
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // ipv4 or ipv6 address family
	hints.ai_socktype = SOCK_STREAM;

	/** Function getaddrinfo returns non-zero if there's a problem */
	if (getaddrinfo(hostname, "http", &hints, &res) != 0)
		throw AddrInfoException("in6_string()");
	
	const int MAX_ATTEMPTS = 15;
	int i = 0;
	for (ptr = res; ptr != NULL; ptr = ptr->ai_next)
	{
		if (MAX_ATTEMPTS <= i++)
		{
			if (ptr->ai_family == AF_INET6)
			{
				addr = &(((struct sockaddr_in6 *)ptr->ai_addr)->sin6_addr);
				inet_ntop(ptr->ai_family, addr, buffer, INET6_ADDRSTRLEN);
				break;
			}
		}
	}

	freeaddrinfo(res);	// free list of address structures
	if (addr == NULL)
	{
		delete [] buffer;
		return NULL;
	}

	return buffer;
}


void send_http(SOCKET s, const char *hostname, const char *method, const char *request = NULL,
	const char *useragent = "cgWinsock App", const char *from = NULL)
{
	std::stringstream reqStream;

	const char *CRLF = "\r\n";

	if (request != NULL)
		reqStream << method << " " << request << " HTTP/1.1" << CRLF;
	else
		reqStream << method << " HTTP/1.1" << CRLF;
	reqStream << "Host: " << hostname << CRLF;
	reqStream << "User-agent: " << useragent << CRLF;
	if (from != NULL) {reqStream << "From: " << from << CRLF;}
	reqStream << "Connection: close" << CRLF << CRLF;
	std::string save = reqStream.str();
	const char *test = save.c_str();
	int size = save.size();

	if(send(s, reqStream.str().c_str(), reqStream.str().size(), 0)
		                                     == SOCKET_ERROR)
		throw SendException("sendHEAD()");
}

/** Assumes that a valid connection has already been established
	on SOCKET s, recv() is called in a loop until the server closes
	the connection (recv returns 0). Received data is read into a
	stream after which memory is allocated and the stream is copied
	when the function returns the total number of bytes received
	the reference *data will point to the received data. */
int recv_data(SOCKET s, char *&data) {
    int totalRecvd = 0;
    std::stringstream sStream;

    while (true) {
        char buffer[MAX_HEADER_SIZE];
        int retValue = recv(s, buffer, MAX_HEADER_SIZE, 0);
        if (retValue == 0)
            break;  // connection has been closed
        else if (retValue == SOCKET_ERROR)
            throw RecvException("http_headreq() unable to receive data");
        else {   
            buffer[retValue] = 0; // append null terminator
            sStream << buffer;    // dump buffer into stream
            totalRecvd += retValue + 1; // tally received bytes
        }
    }

    /** Allocate and read entire read stream into memory */
    data = new char[totalRecvd + 1];
    strcpy_s(data, totalRecvd, sStream.str().c_str());
	data[totalRecvd] = 0;
    return totalRecvd;
}


/** Uses a stream socket with TCP protocol to connect to a remote host given
	a hostname, the function gets a list of address structures and checks
	for valid ipv4 or ipv6 addressess and attempts connections until a valid
	address is found. An HTTP HEAD request is sent and data is streamed.
	Memory is dynamically allocated in resp and should be freed by the caller
	with a call to delete. If the third optional parameter verbose is set 
	true the status of each operation aswell as the head response will be
	printed to standard out */
int http_req(const char *hostname, const char *method, 
	             const char *request, char *&resp, bool verbose = false)
{
	const int MAX_ATTEMPTS = 15;	// Attempts to connect
	struct addrinfo hints, *res, *ptr;	// Address structures and pointer
	int totalRecvd = 0;				// # of bytes received in response

	/** Initialize hints structure */
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // ipv4 or ipv6 address family
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	/** Function getaddrinfo returns non-zero if there's a problem */
	if (getaddrinfo(hostname, "http", &hints, &res) != 0)
		throw WinsockException("getaddrinfo()");
	if (verbose) 
		std::cout << "Log: HTTP HEAD request started..." << std::endl
			 << "Log: Address info structures retrieved for host "
			 << hostname << "..." << std::endl;

	/** Loop through list of address structures and get first valid
		ipv4 or ipv6 address. */
	int i = 0;
	for (ptr = res; (ptr != NULL) && (i++ <= MAX_ATTEMPTS); ptr = ptr->ai_next) {
			if (ptr->ai_family == AF_INET || ptr->ai_family == AF_INET6) {

				/** Create a socket to connect to the host, check for errors */
				SOCKET s = socket(ptr->ai_family, 
					ptr->ai_socktype, ptr->ai_protocol);
				if (s == INVALID_SOCKET)
					throw SocketException("socket()");

				std::cout << "Log: TCP Socket successfully created.." 
					      << std::endl;

				/** Attempt to connect the socket to the host address */
				if (connect(s, ptr->ai_addr, ptr->ai_addrlen) == -1)
					continue; // unable to connect, try another address
				
				/** Get the host IP address as a presentable string */
				char ip[INET6_ADDRSTRLEN];
				inet_ntop(ptr->ai_family, 
					cast_in_addr(ptr->ai_addr), ip, sizeof(ip));
				std::cout << "Log: Connection to " << hostname 
					<< " on address " << ip << " established" 
					<< std::endl << std::endl;

				/** Send the head request and receive the response */
				send_http(s, hostname, method, request, 
					"cgWinsock", "CoryG89@gmail.com");
				totalRecvd = recv_data(s, resp); 

				if (verbose) { 
					std::cout << "Log: Dumping http response...\n\n"
						<< resp << "\nLog: Total number of bytes received: "
						<< totalRecvd << std::endl << std::endl;
				}
				
				/** Output size of data received */
				if (closesocket(s) == -1)
					throw SocketException("Error: unable to close socket");
				break;
			}
	}
	freeaddrinfo(res);
	return totalRecvd;
}


#endif
