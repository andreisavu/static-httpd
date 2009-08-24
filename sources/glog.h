/*
  (c) GarajCode
  Clasa pentru inregistrare mesaje in fisier si in
  debuger. Este thread-safe.
  Date : 21 iulie 2005
  Last date : 23 februarie 2006
*/

#ifndef GARAJ_LOG
#define GARAJ_LOG

#include "gerror.h"
#include "gmutex.h"

#include <fstream.h>
#include <windows.h>
#include <stdlib.h>

/// Garaj Standard Namespace
namespace GSTD
{
	/// Clasa pentru informatii de log.
	/**
		O clasa pentru gestionarea unui fisier 
		de log. Mesajele scrise pot fi de mai multe
		tipuri. Pentru afisarea unui mesaj foloseste 
		functii gen printf. Mesajele sunt afisate atat
		in fisier cat si in fereastra debugerului.
		Fiecare mesaj este scris pe o singura linie fiind
		precedat de un indicator de tip care permite
		filtrarea usoara a log-ului in cautarea anumitor
		mesaje de eroare aparute.
	*/
	class CLog
	{
	public:
		/// Initializare interna
		CLog( char * file = NULL )
		{
			m_has_file = false;
			if( file )
			{
				m_fout.open( file, ios::out | ios::app );
				m_has_file = true;
				putstring("");
			}
		}
		/// Eliberare resurse
		~CLog()
		{
			if( m_has_file )
			{
				m_fout.close();
			}
		}
		/// Scrie un mesaj obisnuit
		void message( const char *text,... )
		{
			char buffer[512];
			strcpy(buffer,"MESSAGE ");
			va_list arg;
			va_start(arg,text);
			vsprintf( buffer + strlen(buffer) , text,arg );
			va_end(arg);
			try
			{
				putstring( buffer );
			}
			catch( CError error )
			{
				throw error;
			}
		}
		/// Scrie un mesaj de eroare 
		void error( const char *text,...)
		{
			char buffer[512];
			strcpy(buffer,"ERROR ");
			va_list arg;
			va_start(arg,text);
			vsprintf( buffer + strlen(buffer) , text,arg );
			va_end(arg);
			try
			{
				putstring( buffer );
			}
			catch( CError error )
			{
				throw error;
			}

		}
		/// Scrie un mesaj foarte grav - care determina inchiderea aplicatiei
		void fatal( const char *text, ...)
		{
			char buffer[512];
			strcpy(buffer,"FATAL ");
			va_list arg;
			va_start(arg,text);
			vsprintf( buffer + strlen(buffer) , text,arg );
			va_end(arg);
			try
			{
				putstring( buffer );
			}
			catch( CError error )
			{
				throw error;
			}

		}
		// Scrie un mesaj mai important
		void important( const char *text, ... )
		{
			char buffer[512];
			strcpy(buffer,"IMPORTANT ");
			va_list arg;
			va_start(arg,text);
			vsprintf( buffer + strlen(buffer) , text,arg );
			va_end(arg);
			try
			{
				putstring( buffer );
			}
			catch( CError error )
			{
				throw error;
			}
		}

	protected:
		fstream m_fout;			///< Fisierul de log propriu-zis
		bool m_has_file;		///< Fisierul este deschis sau nu
		CMutex m_mutex;			///< Restrictionare acces in aplicatii multithreading

		/// Afiseaza un mesaj
		/**
			Mesajul este scris in consola sau in fisier.
			Functia este thread-safe putand fi folosita
			fara probleme in aplicatii multithreading.
		*/
		void putstring( const char * text )
		{
			// foloseste un mutex pentru acces exclusiv
			try
			{
				m_mutex.lock();
			}
			catch( CError error )
			{
				throw error;
			}

			// scrie mesaj in debuger si in fisier
			OutputDebugString( text );
			OutputDebugString( "\n" );
			if( m_has_file ) m_fout<<text<<endl;

			// elibereaza acces exclusiv
			try
			{
				m_mutex.unlock();
			}
			catch( CError error )
			{
				throw error;
			}
		}

	};	// CLog


};	// namespace GSTD

#endif	// GARAJ_LOG