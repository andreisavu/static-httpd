/*
  (c) GarajCode
  Clasa pentru socket
  Date : 30 iunie 2005
  Last date : 23 februarie 2005
*/

/*
	Change log :
		10 august 2005 - adaugata functia pentru impachetare IP in DWORD
		11 august 2005 - safe_send()
		23 august 2005 - thread-safe multiblock send
		6 septembrie 2005 - schimbare interfata clasa
		23 noiembrie 2005 - data type fix
		23 februarie 2005 - comentarii doxygen
  */

#include <winsock.h>

#include "gsocket.h"

#include <stdio.h>

namespace GN
{

	/// Functie auxiliara care impacheteaza un IP
	DWORD pack_ip( char * ip )
	{
		char buffer[20];
		int aux,m_ip_pack;

		strcpy( buffer, ip);

		char *p = strtok( buffer, ". " );
		aux = atoi(p);
		m_ip_pack = aux;

		p = strtok( NULL, ". " );
		while( p )
		{
			m_ip_pack <<= 8;
			aux = atoi(p);
			m_ip_pack += aux;

			p = strtok( NULL, ". ");
		}		

		return m_ip_pack;
	}

	/// Returneaza adresa locala
	/** 
		Returneaza adresa locala, functia este folosita atunci
		cand se creeaza un socket pentru server si care are
		ca adresa IP adresa locala.
	*/
	int GetLocalAddress(LPSTR lpStr, LPDWORD lpdwStrLen)
	{
		struct in_addr *pinAddr;
		LPHOSTENT	lpHostEnt;
		int			nRet;
		int			nLen;

		//
		// Get our local name
		//
		nRet = gethostname(lpStr, *lpdwStrLen);
		if (nRet == SOCKET_ERROR)
		{
			lpStr[0] = '\0';
			return SOCKET_ERROR;
		}

		//
		// "Lookup" the local name
		//
		lpHostEnt = gethostbyname(lpStr);
		if (lpHostEnt == NULL)
		{
			lpStr[0] = '\0';
			return SOCKET_ERROR;
		}	

		//
		// Format first address in the list
		//
		pinAddr = ((LPIN_ADDR)lpHostEnt->h_addr);
		nLen = strlen(inet_ntoa(*pinAddr));
		if ((DWORD)nLen > *lpdwStrLen)
		{
			*lpdwStrLen = nLen;
			WSASetLastError(WSAEINVAL);
			return SOCKET_ERROR;
		}

		*lpdwStrLen = nLen;
		strcpy(lpStr, inet_ntoa(*pinAddr));
	    return 0;
	}

	/// Initializare
	CSocket::CSocket( SOCKET s ):
		m_socket( s )
	{
		m_ip[0] = 0;		
	}

	/// Creaza socket pentru conexiune TCP/IP
	void CSocket::create()
	{
		m_socket = socket(AF_INET, SOCK_STREAM, 0);

		if( m_socket == INVALID_SOCKET )
		{
			GSTD::CError error;
			set_winsock_error( error );
			throw (GSTD::CError)error;
		}
		
		// gaseste adresa IP locala
		DWORD str_len=20;
		GetLocalAddress(m_ip,&str_len);

		// calculeaza ip-ul impachetat
		m_ip_pack = pack_ip( m_ip );
	}

	/// Inchide socket 
	void CSocket::close()
	{
		try
		{
			m_mutex.lock();
		}
		catch( GSTD::CError error )
		{
			throw error;
		}
		if( m_socket == INVALID_SOCKET) return;
		if( closesocket( m_socket ) == SOCKET_ERROR )
		{
			GSTD::CError error;
			set_winsock_error( error );				
			throw (GSTD::CError)error;
		}
		m_socket = INVALID_SOCKET;
		try
		{
			m_mutex.unlock();
		}
		catch( GSTD::CError error )
		{
			throw error;
		}
	}

	/// Conectare socket la un port
	void CSocket::connect(int port, char *lpServerName)
	{
		try
		{
			m_mutex.lock();
		}
		catch( GSTD::CError error )
		{
			throw error;
		}
		// verifica initializarea socket-ului
		if( m_socket == INVALID_SOCKET ) 
		{
			// initializeaza
			try
			{
				create();
			}
			catch(GSTD::CError error)
			{
				throw (GSTD::CError)error;
			}
		}

		struct hostent *lpHostEntry;
		struct in_addr iaHost;

		iaHost.s_addr = inet_addr(lpServerName);
		if (iaHost.s_addr == INADDR_NONE)
		{
			// Nu a fost IP, poate e nume de server
			lpHostEntry = gethostbyname(lpServerName);
		}
		else
		{
			// Adresa de IP
			lpHostEntry = gethostbyaddr((const char *)&iaHost,sizeof(struct in_addr), AF_INET);
		}
		if (lpHostEntry == NULL)
		{
			GSTD::CError error;
			set_winsock_error( error );
			throw (GSTD::CError)error;
		}
	
		// Seteaza portul pentru conexiune
		SOCKADDR_IN saServer;
		saServer.sin_port = htons((unsigned short)port);
	
		// Umple restul din structura pentru server
		saServer.sin_family = AF_INET;
		saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);

		// Conecteaza la server
		int nRet;
		nRet = ::connect(m_socket, (LPSOCKADDR)&saServer, sizeof(SOCKADDR_IN));
		if (nRet == SOCKET_ERROR)
		{
			GSTD::CError error;
			set_winsock_error( error );
			throw (GSTD::CError)error;
		}
		try
		{
			m_mutex.unlock();
		}
		catch( GSTD::CError error )
		{
			throw error;
		}
	}	

	/// Seteaza socket-ul sa asculte conexiuni la un anumit port
	void CSocket::listen( int port )
	{
		if(m_socket == INVALID_SOCKET)
		{
			try
			{
				create();
			}
			catch(GSTD::CError error)
			{
				throw (GSTD::CError)error;
			}
		}

		struct sockaddr_in server;

		// seteaza informatii
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = INADDR_ANY;

		// asociaza socket cu portul
		if (bind(m_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
		{
			GSTD::CError error;
			set_winsock_error( error );
			throw (GSTD::CError)error;
		}
	
		// seteaza socket-ul pentru a asculta un port
		if ( ::listen(m_socket, 5) == SOCKET_ERROR )
		{
			GSTD::CError error;
			set_winsock_error( error );			
			throw (GSTD::CError)error;	
		}
	}

	/// Accepta o conexiune
	CSocket CSocket::accept()
	{
		SOCKET s;
		SOCKADDR_IN addr;
		int len = sizeof(SOCKADDR_IN);
		s = ::accept(m_socket, (LPSOCKADDR)&addr,&len);

		if( s == INVALID_SOCKET )
		{
			GSTD::CError error;
			set_winsock_error( error );
			throw (GSTD::CError)error;
		}

		CSocket ret(s);
		strcpy(ret.m_ip, inet_ntoa(addr.sin_addr));

		// calculeaza ip-ul impachetat
		ret.m_ip_pack = pack_ip( ret.m_ip );
		
		return ret;
	}

	/// Recepteaza informatii
	/**
		Functia returneaza numarul de octeti cititi sau
		arunca o exceptie in cazul aparitiei unei erori.
	*/
	int CSocket::receive(char *buffer, int len) const
	{
		int ret = recv(m_socket, buffer, len, 0);

		if( !ret || ret == SOCKET_ERROR )
		{
			GSTD::CError error;
			set_winsock_error( error );
			throw (GSTD::CError)error;
		}

		return ret;
	}

	#define MAX_BUFFER_LENGTH 512

	/// Scrie mesaj la u socket
	void CSocket::printf( char * text, ... ) const
	{
		char buffer[ MAX_BUFFER_LENGTH ];

		// formateaza mesaj in buffer
		va_list arg;
		va_start( arg, text );
		vsprintf( buffer , text, arg );
		va_end(arg);
	
		// afiseaza mesaj in consola
		try
		{
			send_text( buffer );
		}
		catch(...)
		{
			throw;
		}
	}

	/// Trimite un text 
	void CSocket::send_text( const char* buffer ) const 
	{
		try
		{
			send( buffer, strlen(buffer) );
		}
		catch(...)
		{
			throw;
		}
	}

	/// Trimite un sir care pot contine orice informatie
	void CSocket::send(const char *buffer, int len) const
	{
		// acces exclusiv
		try
		{
			m_mutex.lock();
		}
		catch( GSTD::CError error )
		{
			throw error;
		}
		int ret = ::send(m_socket, buffer, len, 0);

		if( ret!=len || ret == SOCKET_ERROR)
		{
			GSTD::CError error;
			set_winsock_error( error );
			throw (GSTD::CError)error;
		}
		// eliberare acces
		try
		{
			m_mutex.unlock();
		}
		catch( GSTD::CError error )
		{
			throw error;
		}
	}

	// thread-safe multiblock send
	// functii utile atunci cand se doreste trimiterea mai 
	// blocuri de informatii avand siguranta ca nu a intervenit
	// un alt thread.

	/// Acces exclusiv la trimitere
	/**
		Folosita in special in cazurile in care
		un socket este folosit simultan de mai multe
		thread-uri.
	*/
	void CSocket::std_lock()
	{
		try
		{
			m_stdmtx.lock();
		}
		catch( GSTD::CError error )
		{
			throw error;
		}
	}

	/// Trimite block de mesaj
	/**
		Folositoare in cazul in care mai multe
		thread-uri folosesc simultan acelasi socket.	  
	*/
	void CSocket::std_send( const char * buffer, int len ) const
	{
		int ret = ::send(m_socket, buffer, len, 0);

		if( ret!=len || ret == SOCKET_ERROR)
		{
			GSTD::CError error;
			set_winsock_error( error );
			throw (GSTD::CError)error;
		}
	}

	/// Elibereaza acces exclusiv la trimitere
	void CSocket::std_unlock()
	{
		try
		{
			m_stdmtx.unlock();
		}
		catch( GSTD::CError error )
		{
			throw error;
		}
	}

};	// namespace GN

// (c) GarajCode 2005 - programmed by Savu Andrei