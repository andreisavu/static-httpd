/*
  (c) GarajCode
  Clasa pentru winsock - initializare automata
  Date : 30 iunie 2005
  Last date : 23 februarie 2006
  Change log :
	23 noiembrie 2005 - reference count - prevent reinitialisation
					  - more informations
	23 februarie 2006 - comentarii doxygen - aranjare cod
*/

#ifndef GARAJ_WINSOCK_H
#define GARAJ_WINSOCK_H

#include <winsock.h>
#include "gerror.h"

namespace GN
{
	/// Clasa pentru initializare Winsock
	/**
		Initializarea winsock se face automat fiind necesar
		doar crearea unei instante a acestei clase.
	*/
	class CWinsock
	{
	public:
		CWinsock();
		~CWinsock();

		int max_tcp_sockets() const;
		int max_udp_datagrams() const;
		const char * vendor_info() const;
		const char * description() const;

	protected:
		WSAData m_wsadata;
		static int m_refc;

	};	// CWinsock

	/// Initializeaza structura pentru eroare.
	/**
		Seteaza structura pentru eroare in conformitate 
		cu un cod de eroare winsock.
	*/
	void set_winsock_error( GSTD::CError&, int code=0 );

};	// namespace GN

#endif // GARAJ_WINSOCK_H