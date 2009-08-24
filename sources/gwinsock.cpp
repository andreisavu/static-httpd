/*
  (c) GarajCode
  Clasa pentru winsock - initializare automata
  Date : 30 iunie 2005
  Last date : 23 februarie 2006
  Change log :
	23 noiembrie 2005 - instance count
					  - more informations
	23 februarie 2006 - comentarii doxygen - aranjare cod
*/

#include "gwinsock.h"
#include "gerror.h"

/// Garaj Network Namespace
namespace GN
{
	int CWinsock::m_refc = 0;

	/// Initializare automata Winsock cu verificare de reference count
	CWinsock::CWinsock()
	{
		if( m_refc ) 
		{
			// increment instance count
			m_refc++;
			return;
		}

		// initialise Winsock
		int code = WSAStartup(MAKEWORD(1,1),&m_wsadata);

		// check error code
		if( code )
		{
			GSTD::CError error;
			set_winsock_error( error, code );
			throw (GSTD::CError)error;
		}

		// increment instance count
		m_refc++;
	}

	/// Returneaza numarul maxim de conexiuni simultane
	int CWinsock::max_tcp_sockets() const
	{
		return m_wsadata.iMaxSockets;
	}

	/// Numarul maxim de conexiuni UDP posibile
	int CWinsock::max_udp_datagrams() const
	{
		return m_wsadata.iMaxUdpDg;
	}

	/// Informatii despre vendor
	const char * CWinsock::vendor_info() const 
	{
		return m_wsadata.lpVendorInfo;
	}

	/// Winsock implementation description
	const char * CWinsock::description() const
	{
		return m_wsadata.szDescription;
	}

	/// Eliberare resurse winsock
	CWinsock::~CWinsock()
	{
		// check to see if this is the last instance
		m_refc--;
		if( m_refc ) return;

		// cleanup winsock
		if( WSACleanup() == SOCKET_ERROR)
		{
			GSTD::CError error;
			set_winsock_error( error );
			throw (GSTD::CError)error;
		}
	}

	// Functie auxiliara pentru erori Winsock
	void set_winsock_error( GSTD::CError &error, int code )
	{
		if(!code) error.set_code( WSAGetLastError() );
		else error.set_code( code );

		switch( error.get_code() )
		{
		case WSANOTINITIALISED:
			error.set_text( "Winsock not initialised");
			break;
		case WSAENETDOWN:
			error.set_text( "The network subsystem has failed");
			break;
		case WSAEAFNOSUPPORT:
			error.set_text( "Address family not suported");
			break;
		case WSAEINPROGRESS:
			error.set_text( "A blocking operation in progress");
			break;
		case WSAENOTSOCK:
			error.set_text( "Parameter is not socket");
			break;
		case WSAEMFILE:
			error.set_text( "Reached winsock limit");
			break;
		case WSAENOBUFS:
			error.set_text( "No buffer space avaible");
			break;
		case WSAEPROTONOSUPPORT:
			error.set_text( "Protocol not supported");
			break;
		case WSAEPROTOTYPE:
			error.set_text( "Wrong protocol for this socket");
			break;
		case WSAESOCKTNOSUPPORT:
			error.set_text( "Wrong socket for address family");
			break;
		case WSAEINTR:
			error.set_text( "Call canceled");
			break;
		case WSAEWOULDBLOCK:
			error.set_text( "Socket is marked as nonblocking");
			break;
		case WSAHOST_NOT_FOUND:
			error.set_text( "Host not found");
			break;
		case WSATRY_AGAIN:
			error.set_text( "Host not found or server fail");
			break;
		case WSANO_RECOVERY:
			error.set_text( "Nonrecoverable error occurred");
			break;
		case WSANO_DATA:
			error.set_text( "No data record of requested type");
			break;
		case WSAEFAULT:
			error.set_text( "Invalid argument");
			break;
		case WSAEADDRINUSE:
			error.set_text( "Address in use");
			break;
		case WSAEALREADY:
			error.set_text( "A nonblocking call is in progress");
			break;
		case WSAEADDRNOTAVAIL:
			error.set_text( "Invalid address from local machine");
			break;
		case WSAECONNREFUSED:
			error.set_text( "Attempt to connect was rejected");
			break;
		case WSAEINVAL:
			error.set_text( "Connect on listening socket");
			break;
		case WSAEISCONN:
			error.set_text( "Socket already connected");
			break;
		case WSAENETUNREACH:
			error.set_text( "The network is not accesible");
			break;
		case WSAETIMEDOUT:
			error.set_text( "Attemp to connect time exceded");
			break;
		default:
			error.set_text( "Unknown error");
			break;
		}
	}

};	// namespace GN

// (c) GarajCode 2005 - programmed by Savu Andrei