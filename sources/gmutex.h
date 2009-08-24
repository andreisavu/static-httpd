/*
  (c) GarajCode
  Mutex class - used for thread-safe
  Date : 4 iulie 2005
  Last date : 23 februarie 2006
*/
/*
	Codul pentru mutex exista doar daca proiectul
	este setat pentru multithreading( definit _MT )
  */
#ifndef GARAJ_MUTEX_H
#define GARAJ_MUTEX_H

#include <windows.h>
#include "gerror.h"

/// Garaj Standard Namespace
namespace GSTD
{
	/// Mutex - sincronizare acces resurse
	/**
		Aceasta clasa se dovedeste utila in aplicatii multithreading
		in care aceeasi resursa trebuie sa fie folosita de mai multe 
		thread-uri concurent. Pentru a evita probleme de intergritate
		a datelor accesul la informatie este dat pe rand fiecarui thread.
		Codul din interiorul clasei este definit doar daca se compileaza
		o aplicatie multithreading.
	*/
	class CMutex
	{
	public:
		/// Initializare mutex
		CMutex()
		{
#ifdef _MT
			m_mutex = CreateMutex(NULL, false, NULL);
			if( !m_mutex )
			{
				CError error( GetLastError(), "Mutex creation error");
				throw error;
			}
#endif
		}

		/// Eliberare mutex
		~CMutex()
		{
#ifdef _MT
			m_mutex = NULL;
#endif
		}

		/// Blocare mutex - atunci cand un thread cere acces exclusiv la o resursa
		void lock() const
		{
#ifdef _MT
			if( WaitForSingleObject( m_mutex, INFINITE ) == WAIT_FAILED )
			{
				CError error( GetLastError(), "Error locking mutex");
				throw error;
			}			
#endif
		}

		/// Elibereare mutex - resursa devine libera 
		void unlock() const
		{
#ifdef _MT
			if( ReleaseMutex( m_mutex ) == 0 )
			{
				CError error( GetLastError(), "Error releasing mutex");
				throw error;
			}
#endif
		}
			
	protected:
#ifdef _MT
		HANDLE m_mutex;			///< Variabila de tip mutex
#endif
	
	};	// CMutex

};	// namespace gstd

#endif	// GARAJ_MUTEX_H
