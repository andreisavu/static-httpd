/*
  (c) GarajCode 2005
  Simple Multithreaded/Multiclient server
  Date : 30 iunie 2005
  Last date : 23 februarie 2006
*/

/*
	Change log :
		10 august 2005 - adaugat suport pentru optiuni de securitate ( MCSC )
		11 august 2005 - suport pentru trusted ip
					   - suport pentru banned ip
		23 august 2005 - finisari
		6 septembrie 2005 - modificare interfata clasa
		7 septembrie 2005 - utilizare arbori binari la cautare IP
		31 octombrie 2005 - WIN9X_HACK
		23 februarie 2006 - comentarii doxygen - mici schimbari de interfata
  */

#include "gserver.h"

#include <stdio.h>
#include <process.h>

// dezactiveaza avertizarile pentru STL
#pragma warning(disable:4786)
#pragma warning(disable:4786)

// proiectul threbuie setat pentru aplicatii multithread
#ifndef _MT		
#error Please set your compiler to generate Multithreaded aplications.
#endif

#define CLOSE_WAIT_TIME 500		///< Timpul astepta inainte ca thread-ul listen sa fie inchis fortat

/// Garaj Network Namespace
namespace GN
{
	/// Cautare caracter in sir
	inline int is_in( char * buffer, char ch )
	{
		char *p = buffer;

		while( *p )
		{
			if( *p == ch ) return 1;
			p++;
		}
		return 0;
	}

	/// Functie auxiliara folosita pentru impartire textelor
	int get_token( char * buffer, char * text, int start, int end, char * sep )
	{
		int k=0;
		int i = start;

		// trece peste caracterele initiale
		while( is_in(sep, text[i]) && i<end ) i++;
		
		// transfera continutul in buffer
		while( !is_in(sep, text[i]) && i < end ) 
		{
			buffer[k++] = text[i];
			i++;
		}
		buffer[k] = 0;

		return i;
	}

	/// Initializare server
	CServer::CServer() :
		m_listen_thread( NULL )
	{
	}

	/// Inchidere automata server
	CServer::~CServer()
	{
		close();
	}

	/// Adauga lista de ip de incredere
	/**
		Adauga lista de IP-uri a caror conectare 
		este permisa. IP-urile din lista sunt separate 
		prin ;. Filtrarea este facuta automat de server.
	*/
	void CServer::add_tiplist( char * list )
	{
		// proceseaza lista de ip-uri si adauga
		// in lista de ip-uri de incredere
		char buffer[50];
		char sep[] = "; ";
		int l = strlen( list );
		int st = -1;

		if( !l ) return;
		
		do
		{
			st++;
			st = get_token( buffer, list, st, l, sep);
			
			if( strlen(buffer)>= 7 ) add_trusted_ip( pack_ip(buffer) );

		}while( st != l );		
	}

	/// Adauga lista de ip-uri interzise
	/**
		Adauga lista de IP-uri a caror conectare este
		interzisa. Trebuie sa fie separate prin ;. 
		Filtrarea este facuta transparent de catre server.
	*/
	void CServer::add_bannedlist( char * list )
	{
		char buffer[50];
		char sep[] = "; ";
		int l = strlen( list );
		int st = -1;

		if( !l ) return;
		
		do
		{
			st++;
			st = get_token( buffer, list, st, l, sep);
			
			if( strlen(buffer)>= 7 ) add_banned_ip( pack_ip(buffer) );

		}while( st != l );
	}

	/// Informatii pentru thread-ul server
	/**
	 Informatii necesare pentru thread-ul de server
	 structura este folosita doar intern pentru
	 trimiterea de informatii intre thread-uri.
	*/
	struct ServerData
	{
		ClientList *client_list;				///< Lista de clienti conectati
		CSocket *socket;						///< Server or client socket

		SERVER_PROCEDURE action;				///< Functia pentru client
		SERVER_EVENT_HANDLER event_handler;		///< Gestionare evenimente interne server

		UINT max_client_number;					///< Numarul maxim de clienti conectati simultan
		UINT mcsc;								///< Accepta sau nu mai multe conexiuni de la acelasi IP simultan

		UINT utip;								///< Foloseste sau nu lista de IP-uri de incredere
		TrustedIPList *tip_list;				///< Lista de IP-uri de incredere impachetate sub forma de numere 

		UINT ubip;								///< Foloseste sau nu lista de IP-uri interzise
		BannedIPList *bip_list;					///< Lista de IP-uri a caror conectare nu este permisa

	};

	/// Client thread
	/**
		Serverul se bazeaza pe o arhitectura multiclient\multithreading si
		din acest motiv fiecare nou client este gestiona de un thread 
		separat. Acest tip de arhitectura desi mai putin eficient datorita
		resurselor folosite de thread-uri si timpului necesar pentru crearea 
		unui nou thread permite implementarea foarte usoara a protocoalelor de 
		comunicatii. Codul pentru comunicatii este furnizat de programator
		printr-o functie externa serverului. Aici este apelata acea functie.
	*/
	DWORD WINAPI ClientThread( LPVOID param )
	{
		ServerData *data=(ServerData*)param;

		// apeleaza functia care sa gestioneze clientul
		if(data->action)
		{
			try
			{
				data->action(*(data->client_list),*(data->socket));
			}
			catch( GSTD::CError error )
			{
				if( data->event_handler )
				{
					// semnaleaza eroare aparuta in functia de protocol
					data->event_handler( SM_CLIENT_ERROR, &error, data->socket );
				}
			}
		}
		
		// extrage adresa din lista
		data->client_list->remove( data->socket );

		// semnaleaza deconectare client
		if( data->event_handler )
		{
			data->event_handler( SM_CLIENT_DISCONNECTED, NULL, data->socket);
		}

		// inchide socket-ul
		try
		{
			data->socket->close();
		}
		catch( GSTD::CError error)
		{
			if( data->event_handler )
			{
				// semnaleaza eroare la inchidere
				data->event_handler( SM_CLIENT_ERROR, &error, data->socket );
			}
		}

		// elibereaza memoria folosita de client
		delete data->socket;
		delete data;

		return 0;
	}

#ifdef WIN9X_HACK
	// necesara deoarece exista o problema cu 
	// functia CreateThread pe win9x - returneaza NULL
	void ClientThread_Hack( void * param )
	{
		ClientThread( param );
	}	
#endif


	/// Listen Thread
	/**
		Acesta este thread-ul server. Atunci cand apare un nou client
		aceasta functie creeaza un nou thread pentru a-l gestiona. Filtrarea
		conexiunilor dupa IP se face aici. Orice fel de eroarea aparute 
		in interiorul acestei functii este semnalata folosind functia 
		furnizata ca parametru pentru evenimente de la server.
	*/	
	DWORD WINAPI ListenThread( LPVOID param )
	{
		ServerData *data=(ServerData*)param;
		ServerData *client;
		CSocket aux;

		// semnaleaza pornirea daemonului
		if( data->event_handler )
		{
			data->event_handler( SM_CREATE, NULL, data->socket);
		}
		
		while( true )
		{
			// asteapta o conexiune
			try
			{
				aux = data->socket->accept();
			}
			catch(GSTD::CError error)
			{
				if( data->event_handler)
				{
					// semnaleaza problema aparuta
					data->event_handler( SM_CONNECTION_ERROR, &error, NULL);
				}
				break;
			}

			// use banned ip
			if( data->ubip )
			{
				// verifica daca ip-ul se afla in lista 
				// de ip-uri interzise
				if( data->bip_list->count( aux.get_ip_pack() ) )
				{
					// semnaleaza tentativa de conectare de la un IP
					// din lista de interzis
					if( data->event_handler )
					{
						data->event_handler( SM_BIP_ERROR, NULL, &aux );
					}
					
					aux.close();

					continue;
				}
			}

			// use trusted ip
			if( data->utip )
			{
				// verifica daca ip-ul este in lista 
				// daca nu a fost gasit conexiunea este inchisa si se 
				// asteapta alt client
				if( !data->tip_list->count( aux.get_ip_pack() ) )
				{
					// semnaleaza eroare
					if( data->event_handler )
					{
						data->event_handler( SM_TIP_ERROR, NULL, &aux );
					}

					aux.close();

					continue;
				}
			}

			// multiple connexions from same client
			if( !data->mcsc )
			{
				// verifica daca acest client mai este conectat odata

				for( ClientListIterator it = data->client_list->begin();
					 it != data->client_list->end(); it++) 
					 if( aux.get_ip_pack() == (*it)->get_ip_pack() )
					 {
						 // semnaleaza eroare
						 if( data->event_handler )
						 {
							 data->event_handler( SM_MCSC_ERROR, NULL, &aux );
						 }

						 // inchide conexiune
						 aux.close();
						 break;
					 }

				// daca a fost gasit se asteapta o alta conexiune
				if( it != data->client_list->end() ) continue;
			}

			// verifica limita pentru clienti
			if( data->client_list->size() >= data->max_client_number ) 
			{
				// semnaleaza depasire limita de conexiuni simultane
				if(data->event_handler)
				{
					data->event_handler(SM_CLIENT_LIMIT_REACHED, NULL, &aux);
				}
				// refuza conexiunea curenta
				try
				{
					aux.close();
				}
				catch( GSTD::CError error )
				{
					if( data->event_handler )
					{
						// semnaleaza eroare
						data->event_handler( SM_CLIENT_ERROR, &error, &aux);
					}
				}
				// asteapta pentru urmatorul client
				continue;
			}

			// semnaleaza conectarea unui nou client si asteapta acceptul conexiunii
			if( data->event_handler )
			{
				if( !data->event_handler( SM_CLIENT_CONNECTED, NULL, &aux ) )
				{
					// conexiunea a fost refuzata
					try
					{
						aux.close();
					}
					catch( GSTD::CError error )
					{
						if( data->event_handler )
						{
							// semnaleaza eroare
							data->event_handler( SM_CLIENT_ERROR, &error, &aux);
						}
					}

					// asteapta urmatorul client
					continue;
				}
			}

			// aloca memorie pentru un nou client
			if( !(client = new ServerData) ) 
			{
				// problema interna cu memoria - server-ul nu trebuie inchis
				// semnaleaza problema
				if(data->event_handler)
				{
					data->event_handler(SM_NO_MEMORY_FOR_CLIENT, NULL, &aux);
				}
				// refuza conexiunea curenta din lipsa de resurse
				try
				{
					aux.close();
				}
				catch( GSTD::CError error )
				{
					if( data->event_handler )
					{
						// semnaleaza problema
						data->event_handler( SM_CLIENT_ERROR, &error, &aux);
					}
					continue;
				}

			}

			// seteaza structura
			client->action = data->action;
			client->client_list = data->client_list;
			client->event_handler = data->event_handler;

			// aloca memorie pentru socket 
			if( !(client->socket = new CSocket) ) 
			{
				// problema cu memoria - se renunta la acest client
				if(data->event_handler)
				{
					data->event_handler(SM_NO_MEMORY_FOR_CLIENT, NULL, &aux);
				}
				try
				{
					aux.close();
				}
				catch( GSTD::CError error )
				{
					if( data->event_handler)
					{
						// problema de semnalat
						data->event_handler( SM_CLIENT_ERROR, &error, &aux); 
					}
				}
				delete client;
				continue;
			}
			*client->socket = aux;

			// salveza adresa noului socket in lista
			data->client_list->push_back(client->socket);

#ifndef WIN9X_HACK
			// creaza un nou thread pentru client
			HANDLE tmph = CreateThread(NULL,0,ClientThread,client,0,NULL);

			if( !tmph )
			{
				// nu s-a putut crea thread-ul pentru gestionarea
				// clientului, conexiunea este refuzata si server-ul
				// isi vede de treaba mai departe eroare fiind semnalata
				if( data->event_handler)
				{
					data->event_handler( SM_THREAD_ERROR, NULL, &aux);
				}
				try
				{
					aux.close();
				}
				catch( GSTD::CError error )
				{
					if( data->event_handler )
					{
						// semnaleaza problema
						data->event_handler( SM_CLIENT_ERROR, &error, &aux );
					}
				}

				// elibereaza memoria alocata pentru client
				delete client->socket;
				delete client;
				
				// asteapta urmatoarea conexiune
				continue;
			}
			// inchide handle
			CloseHandle( tmph );
#endif

#ifdef WIN9X_HACK // problema legat de CreateThread - un hack pentru WIN9X
			if( _beginthread( ClientThread_Hack, 0, client ) == -1L )
			{
				// nu s-a putut crea thread-ul pentru gestionarea
				// clientului, conexiunea este refuzata si server-ul
				// isi vede de treaba mai departe eroare fiind semnalata
				if( data->event_handler)
				{
					data->event_handler( SM_THREAD_ERROR, NULL, &aux);
				}
				try
				{
					aux.close();
				}
				catch( GSTD::CError error )
				{
					if( data->event_handler )
					{
						// semnaleaza problema
						data->event_handler( SM_CLIENT_ERROR, &error, &aux );
					}
				}

				// elibereaza memoria alocata pentru client
				delete client->socket;
				delete client;
				
				// asteapta urmatoarea conexiune
				continue;
			}
#endif
			
		}

		// inchide toate conexiunile cu clientii si asteapta 
		// ca lista sa devina vida
		for(ClientListIterator it = data->client_list->begin();
			it != data->client_list->end(); it++)
			{
				try
				{
					(*it)->close();
				}
				catch( GSTD::CError error )
				{
					if( data->event_handler )
					{
						//semnaleaza eroare
						data->event_handler( SM_CLIENT_ERROR, &error, *it);
					}
				}
			}

		// asteapta pana toti clienti se inchid
		// verificare se face la un interval de jumatate
		// de secunda pentru a evita utilizarea inutila
		// a procesorului
		while( data->client_list->size() ) Sleep(500);
				
		// semnaleaza inchiderea daemonului
		if( data->event_handler )
		{
			data->event_handler( SM_CLOSE, NULL, data->socket);
		}

		// elibereaza memoria necesara pentru daemon
		delete data;

		return 1;
	}		

#ifdef WIN9X_HACK
	// problema cu CreateThread pe WIN9X
	void ListenThread_Hack( void * param )
	{
		ListenThread( param );
	}
#endif


	/// Initializarea serverulu
	/**
		Aceasta functia initializeaza toate structurile interne
		pentru un nou server dupa care creeaza thread-ul pentru 
		ascultarea de noi conexiuni. Dupa activare functia se termina
		iar serverul ruleaza in paralel.
	*/
	void CServer::create( ServerCreationData &init_data )
	{
		// verifica daca avem o functie pentru client
		if( !init_data.server_procedure )
		{
			throw GSTD::CError(0, "No client function" );
		}

		// initializeaza socket-ul de server
		try
		{
			m_listen_socket.create();
			m_listen_socket.listen( init_data.port );
		}
		catch(GSTD::CError error)
		{
			throw (GSTD::CError)error;
		}

		// aloca memorie necesara pentru server
		ServerData *data;
		if(!(data=new ServerData))
		{
			m_listen_socket.close();

			throw GSTD::CError( GSTD::BAD_ALLOC );
		}

		// seteaza structura care va fi pasata mai departe
		data->action = init_data.server_procedure;				// functia pentru protocol
		data->event_handler = init_data.server_event_handler;	// gestionare evenimente interne
				
		data->client_list = &m_client_list;						// lista de clienti
		data->socket = &m_listen_socket;						// socket-ul server

		data->mcsc = init_data.mcsc;							// conexiuni multiple de la acelasi IP
		data->max_client_number = init_data.max_client_number;	// numarul maxim de conexiuni simultane

		data->utip = init_data.utip;							// use trusted ip list
		data->tip_list = &m_tip_list;								// lista de ip-uri de incredere

		data->ubip = init_data.ubip;							// use banned ip list
		data->bip_list = &m_bip_list;						// lista de ip-uri interzise
		
		// porneste daemonul pentru server 

#ifndef WIN9X_HACK

		// versiunea care TEORETIC trebuie sa fie independenta de 
		// versiunea de Windows
		m_listen_thread = CreateThread(NULL, 0, ListenThread, data, 0, NULL);

		if( !m_listen_thread )
		{
			// eroare la pornire daemon
			GSTD::CError error;
			error.set_code( GetLastError() );
				
			LPVOID lpMsgBuf;
 			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				error.get_code(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
	
			// Fill error struct
			error.set_text( (char*)lpMsgBuf );

			// Free the buffer.
			LocalFree( lpMsgBuf );

			// elibereaza memoria alocata
			delete data;

			// semnaleaza eroare
			throw (GSTD::CError)error;
		}				
#endif

#ifdef WIN9X_HACK

	// foloseste _beginthread 
	// dezavantaj major - nu mai permite folosirea WaitForSingleObject
	if( _beginthread( ListenThread_Hack, 0, data ) == -1L )
	{
		throw GSTD::CError( 0, "Error creating listen thread - WIN9XHACK" );
	}
#endif

	}


	/// Inchide serverul
	/**
		Pentru a inchide serverul este inchis socketul pentru server.
		Pentru a fi sigur ca toate resursele au fost eliberate corect
		si nu a ramas memorie alocata serverul nu se inchide pana
		in momentul in care toate thread-urile clientilor sau inchis
		( dupa ce in prealabil socketurile clientilor au fost inchise ).
	*/
	void CServer::close()
	{
		try
		{
			m_listen_socket.close();				
		}
		catch(GSTD::CError error)
		{
			throw (GSTD::CError)error;
		}

#ifndef WIN9X_HACK
		if(m_listen_thread) 
		{
			WaitForSingleObject(m_listen_thread, INFINITE);
			m_listen_thread = NULL;
		}
#endif

#ifdef WIN9X_HACK

		// asteapta pana ce toti clientii se inchide
		while( m_client_list.size() ) Sleep( 500 );
		Sleep( CLOSE_WAIT_TIME );
#endif
		
		// elibereaza lista de ip in care se are incredere
		m_tip_list.clear();

		// elibereaza lista de ip-uri interzise
		m_bip_list.clear();
	}

};	// namespace GN

// (c) GarajCode 2005-2006 - programmed by Savu Andrei