/*
  (c) GarajCode
  Clasa pentru socket
  Date : 30 iunie 2005
  Last date : 23 februarie 2006
*/

/*
	Change log :
		10 august 2005 - adaugata functia pentru impachetare ip in DWORD
		11 august 2005 - safe_send()
		23 august 2005 - thread-safe multiblock send
		6 septembrie 2005 - schimbare interfata clasa
						  - aranjare cod
		23 noiembire 2005 - data type fix
		23 februarie 2006 - comentarii doxygen - aranjare cod
  */

#ifndef GARAJ_SOCKET_H
#define GARAJ_SOCKET_H

#include "gwinsock.h"
#include "gerror.h"
#include "gmutex.h"

/// Garaj Network Namespace
namespace GN
{
	/// Clasa pentru socket
	/**
		Aceasta clasa abstractizeaza si mai mult conceptul
		de socket. Datorita multor functii ajutatoare folosirea
		unui socket pentru TCP/IP devine foarte usoara si intuitiva.
		Orice eroarea aparuta la trimiterea datelor este semnalata
		printr-o exceptie. Un server care sa accepte o singura 
		conexiune poate fi creat foarte usor.
	*/
	class CSocket
	{
	public:
		// initializare
		CSocket(SOCKET s=INVALID_SOCKET);

		// creaza socket pentru conexiune TCP/IP 
		void create();
		// inchide socket-ul
		void close();

		// se conecteaza la un host dat prin nume sau ip in forma xxx,xxx,xxx,xxx
		void connect(int port,char *host_or_ip);

		// seteaza socket ca server pentru un anumit port
		void listen(int port);
		// accepta o conexiune 
		CSocket accept();

		// recepteaza informatii in buffer 
		int receive(char *buffer,int len) const; 
		
		// trimite informatii
		void send(const char *buffer,int len) const;
		void send_text( const char *buffer ) const;
		void printf( char *,...) const;
		
		// thread-safe multiblock send
		void std_lock();
		void std_unlock();
		void std_send( const char * buffer, int len ) const; 

		// adresa ip a socket-ului
		const char *get_ip() const { return m_ip; }
				
		// returneaza ip-ul impachetat intr-un DWORD
		DWORD get_ip_pack() const { return m_ip_pack; }

		/// Trimitere date in retea - structuri statice
		/**
			Acest operator poate fi folosit numai pentru trimiterea
			structurilor de date alocate static in retea. Datoria
			faptului ca foloseste template-uri poate fi folosita 
			pentru orice. Atentie! Daca sunt trimise date alocate
			dinamic rezultatele sunt imprevizibile.
		*/
		template<class T> 
		const CSocket & operator << (const T value) const
		{
			try
			{
				send((char*)&value, sizeof(value));
			}
			catch(GSTD::CError error)
			{
				throw (GSTD::CError)error;
			}

			return *this;
		}

		/// Receptioneaza date din retea - structuri statice
		/**
			Operatorul poate fi folosit pentru a receptiona
			date alocate static din reta. 
		*/
		template<class T>
		const CSocket & operator >> (T &value) const
		{
			try
			{
				receive((char*)&value, sizeof(value));
			}
			catch(GSTD::CError error)
			{
				throw (GSTD::CError)error;
			}
			
			return *this;
		}
		
	protected:
		char m_ip[20];			///< Adresa IP a socketului
		DWORD m_ip_pack;		///< Adresa IP impachetata intr-un numar
		SOCKET m_socket;		///< Socket-ul real
		GSTD::CMutex m_mutex;	///< Pentru thread-safe, nu stiu daca este chiar necesar
		GSTD::CMutex m_stdmtx;	///< Mutex pentru thread-safe multiblock send

	};	// CSocket

	// functie auxiliara care impacheteaza un IP
	DWORD pack_ip( char * ip );

};	// namespace GN

#endif // GARAJ_SOCKET_H

// (c) GarajCode 2005-2006 - programmed by Savu Andrei