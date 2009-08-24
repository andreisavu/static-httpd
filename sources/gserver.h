/*
  (c) GarajCode 2005
  Multithreaded server
  Date : 30 iunie 2005
  Last date : 23 februarie 2006
*/

/*
	Change log :
		10 august 2005 - suport pentru gestionare MCSC
		11 august 2005 - suport pentru trusted ip
  				       - suport pentru banned ip
		6 septembrie 2005 - modificare interfata clasa
		7 septembrie 2005 - utilizare arbori binari la cautare IP
		31 octombrie 2005 - win9x hack - problema cu create thread 
							using define WIN9X_HACK - _beginthread();
		23 februarie 2006 - comentarii doxygen - mici modificari interfata
*/

#ifndef GARAJ_SERVER_H
#define GARAJ_SERVER_H

#include "gsocket.h"

/*
	Daca programul este compilat pe un sistem de operare Win9x
	sau se doreste rularea programului pe un astfel de sistem
	trebuie sa definesti WIN9X_HACK pentru a evita o
	problema care apare la folosirea functiei CreateThread
  */
// #define WIN9X_HACK	

// dezactiveaza avertizarile pentru STL
#pragma warning(disable:4786)
#pragma warning(disable:4786)

#include <list>
#include <set>

/// Garaj Network Namespace
namespace GN
{
	class CServer;

	/// Lista in care sunt retinuti clientii
	/**
		Orice client conectat este adaugat in lista
		interna mentinuta de server. In momentul
		in care conexiunea este inchisa socketul
		este scos din lista.
	*/
	typedef std::list<CSocket *> ClientList;

	/// Iterator pentru lista de clienti
	typedef std::list<CSocket *>::iterator ClientListIterator;

	// Functiile necesare pentru server
	

	/// Functia pentru gestionarea clientului
	/**
		Functia responsabila pentru protocolul de comunicare cu clientul.
		Fiecare client conectat este pasat acestei functii pentru a fi 
		gestionat. orice eroare poate fi semnalata printr-o exceptie
		care va fi semnalata ca un eveniment SM_CLIENT_ERROR in event handler.
		Functia primeste ca parametru lista de clienti pentru a permite 
		comunicarea usoara cu ceilalti client - broadcast.
	*/
	typedef void (*SERVER_PROCEDURE)(ClientList &,CSocket &);

	/// Functie pentru gestionare evenimente aparua in server.
	/**
		Aceasta functie primeste informatii despre orice fel de eveniment
		care are loc la nivelul server-ului. In cazul mesajului SM_CLIENT_CONNECTED
		valoarea de return a acestei functii determina daca clientul este
		acceptat sau conexiunea este inchisa imediat. Functia este optionala.
	*/
	typedef bool (*SERVER_EVENT_HANDLER)(UINT msg , GSTD::CError *error, CSocket *);


	// lista de ip in care se are incredere si carora le este permisa conectarea
	typedef std::set<DWORD> IPList;		///< Lista de IP-uri in forma impachetata
	typedef IPList  TrustedIPList;		///< IP-uri a caror conectare este permisa
	typedef IPList  BannedIPList;		///< IP-uri a caror conectare nu este permisa


	// iterator pentru lista de IP in care se are incredere
	typedef std::set<DWORD>::iterator  IPListIterator;		///< Iterator pentru lista de IP-uri
	typedef IPListIterator  TrustedIPListIterator;			///< Iterator pentru lista de IP-uri in care se are incredere
	typedef IPListIterator  BannedIPListIterator;			///< Iterator pentru lista de IP-uri a caror conectare este interzisa


	/// Mesaje trimise de server
	enum SERVER_MESSAGE
	{
		SM_CLIENT_CONNECTED,		///< S-a conectat un nou client - valoarea de return determina daca este acceptat sau nu
		SM_CLIENT_DISCONNECTED,		///< Un client s-a deconectat sau a fost deconectat fortat

		SM_MCSC_ERROR,				///< S-a incercat o a doua conectarea la un server cu mcsc true
		SM_TIP_ERROR,				///< S-a incercat conectarea de la un IP in care nu se are incredere
		SM_BIP_ERROR,				///< S-a incercat conectarea de la un IP care nu are voie sa se conecteze

		SM_CLIENT_ERROR,			///< Eroare nedefinita la client 
		SM_CLIENT_LIMIT_REACHED,	///< S-a atins limita maxima de clienti conectati simultan
		SM_NO_MEMORY_FOR_CLIENT,	///< Nu se poate aloca memorie pentru client
		SM_THREAD_ERROR,			///< Nu se poate creea thread-ul pentru client
		
		SM_CONNECTION_ERROR,		///< Eroare in functia de acceptare a conexiunii
		
		SM_CREATE,					///< S-a creat thread-ul pentru acceptare conexiuni
		SM_CLOSE					///< Serverul urmeaza sa se inchida
	};
	

	/// Informatiile necesare pentru a crea un nou server
	/**
		Orice server nou creat trebuie sa primeasca o functie
		pentru gestionarea clientilor, optional o functie
		pentru evenimente interne, un numar de port, numarul
		maxim de conexiuni simultane acceptate, liste de IP-uri
		de incredere sau de IP-uri a caror conectare este interzisa.
	*/
	class ServerCreationData
	{
	public:

		/// Initializare cu valori standard
		ServerCreationData()
		{
			server_procedure = NULL;		
			server_event_handler = NULL;
			port = 4572;					 
			max_client_number = 128;		
			mcsc = 1;								// setarea standard
			utip = 0;								// standard sunt acceptate conexiune de la orice IP
			ubip = 0;						
		}

		SERVER_PROCEDURE server_procedure;			///< Functie furnizata de programator care este apelata atunci cand se conecteaza un nou client
		SERVER_EVENT_HANDLER server_event_handler;	///< Orice mesaj de la server este trimis acestei functii

		UINT port;									///< Portul la care serverul asculta cereri de conectare

		UINT max_client_number;						///< Numarul maxim de clienti conectati simultan
		UINT mcsc;									///< Conexiuni multiple de la acelasi IP

		UINT utip;									///< Se folosesc sau nu IP-uri de incredere 
		UINT ubip;									///< Se folosesc sau nu IP-uri interzise
	};

	/// Clasa pentru server
	/**
		Aceasta clasa a fost scrisa in ideea de a face cat mai simplu cu putinta
		crearea unui server multiclient. Arhitectura pe care se bazeaza acest 
		server este una multithreading. Fiecare client nou este gestionat de un
		thread separat. Desi putin ineficient acest mecanism de gestionare a clientilor
		permite o implementarea foarte usoara a protocolului de comunicatie. Orice
		fel de eroare este gestionata folosind exceptii. Serverul include si posibilitati
		de filtrare interna a comunicatiilor dupa IP. Usurinta in folosire si initializare
		face ca aceasta clasa sa fie un instrument ideal pentru orice incepator.
	*/
	class CServer
	{
	public:
		CServer();
		~CServer();

		// creaza sau inchide serverul
		void create( ServerCreationData &server_data );
		void close();		
		
		// adauga un IP in lista - trebuie sa fie in forma impachetata
		void add_trusted_ip( DWORD ip ) { m_tip_list.insert(ip); }
		void remove_trusted_ip( DWORD ip ) { m_tip_list.erase(ip); }

		// adauga IP oprit la conectare
		void add_banned_ip( DWORD ip ) { m_bip_list.insert(ip); }
		void remove_banned_ip( DWORD ip ) { m_bip_list.erase(ip); }

		// adauga lista de ip-uri
		void add_tiplist( char * list );
		void add_bannedlist( char *list );

		// citeste ip
		const char *get_ip() { return m_listen_socket.get_ip(); }

		// returneaza lista de clienti
		ClientList & get_client_list() { return m_client_list; }

	protected:
		CSocket m_listen_socket;		///< Socket folosit pentru server
		HANDLE m_listen_thread;			///< Thread-ul folosit pentru ascultare
				
		ClientList m_client_list;		///< Lista clientilor conectati
		TrustedIPList m_tip_list;		///< Lista de IP-uri a caror conectare este permisa
		BannedIPList m_bip_list;		///< Lista de IP-uri a caror conectare este interzisa

	};

};	// namespace GN

#endif // GARAJ_SERVER_H

// (c) GarajCode 2005 - programmed by Savu Andrei