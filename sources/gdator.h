/*
  (c) GarajCode  2005-2006 Savu Andrei
  Structura generala pentru a retine informatii
  Date : 20 iulie 2005
  Last date : 23 februarie 2006
*/

#ifndef GARAJ_DATOR_H
#define GARAJ_DATOR_H

#include "gerror.h"

#include <stdlib.h>

#include <ctype.h>

/// Garaj Standard Namespace 
namespace GSTD
{
	/// Tipuri de informatii retinute
	enum DATOR_TYPE
	{
		EMPTY_DATOR,		///< Nici un fel de informatie.
		INT_DATOR,			///< Numar intreg
		FLOAT_DATOR,		///< Numar real
		STRING_DATOR,		///< Text
		BINARY_DATOR		///< Date binare
	};
	
	/// O clasa generala pentru stocare de informatii
	/**
	Clasa implementeaza un tip de variabila care 
	poate contine orice fel de date. 
	*/
	class CDator
	{
	public:
		/// Initializare variabile interne
		CDator()
		{
			m_type = EMPTY_DATOR;
			m_size = 0;
			m_data = NULL;
		}
		/// Constructor de copiere
		CDator( const CDator & d )
		{
			m_type = EMPTY_DATOR;
			m_size = 0;
			m_data = NULL;
			try
			{
				set( d.m_size, d.m_data );
			}
			catch( GSTD::CError error )
			{
				throw error;
			}
			m_type = d.m_type;
		}
		/// Eliberare memorie folosita
		~CDator()
		{
			release();
		}
		/// Este gol ?
		bool empty()
		{
			if( m_type == EMPTY_DATOR ) return true;
			return false;
		}
		/// Elibereaza memorie folosita
		void release()
		{
			if( m_type != EMPTY_DATOR )
			{
				delete[] m_data;
				m_data = NULL;
				m_type = EMPTY_DATOR;
				m_size = 0;
			}
		}
		/// Operatorul de atribuire
		CDator & operator = ( const CDator & d)
		{
			if( this == &d ) return *this;
			try
			{
				set( d.m_size, d.m_data );
			}
			catch( GSTD::CError error )
			{
				throw error;
			}
			m_type = d.m_type;
			return *this;
		}

		/// Memoreaza intreg
		void set(int value) 
		{
			try
			{
				set( sizeof(int), (char*)&value);
			}
			catch( GSTD::CError error )
			{
				throw error;
			}
			m_type = INT_DATOR;
		}
		/// Memoreaza float
		void set(float value) 
		{
			try
			{
				set( sizeof(float), (char*)&value);
			}
			catch( GSTD::CError error )
			{
				throw error;
			}
			m_type = FLOAT_DATOR;
		}
		/// Memoreaza sir de caractere terminat cu zero
		void set(char *string)
		{
			try
			{
				if( string ) set( strlen(string)+1, string);
			}
			catch( GSTD::CError error )
			{
				throw error;
			}
			if( string ) m_type = STRING_DATOR;
		}
		/// Seteaza pentru date binare - folositor la memorarea structurilor
		void set(int size, char *data)
		{
			if( size == m_size )
			{
				memcpy( m_data, data, size);
				m_type = BINARY_DATOR;
			}
			else
			{
				release();

				m_size = size;

				// aloca memorie
				if( !(m_data = new char[ m_size ]) )
				{
					GSTD::CError error( BAD_ALLOC );
					throw error;
				}

				memcpy( m_data, data, m_size );

				m_type = BINARY_DATOR;
			}
		}
	
		/// Converteste un string la tipul cel mai apropiat
		void convert( char *text )
		{
			// analizeaza un string si converteste corespunzator
			int type = is_number( text );

			switch( type )
			{
			case INT_DATOR:
				set(atoi(text));
				break;
			case FLOAT_DATOR:
				set((float)atof(text));
				break;
			case STRING_DATOR:
				set(text);
			}
		}

		/// Returneaza dimensiunea
		int get_size()
		{
			return m_size;
		}
		/// Returneaza tipul
		int get_type()
		{
			return m_type;
		}

		/// Converteste la integer
		operator int()
		{
			float aux;
			switch( m_type )
			{
			case INT_DATOR:
				return (*(int*)m_data);
			case FLOAT_DATOR:
				aux = (*(float*)m_data);
				return (int)aux;
			default:
				return 0;
			}
		}

		/// Converteste la float
		operator float()
		{
			switch( m_type )
			{
			case INT_DATOR:
				return (float)(*(int*)m_data);
			case FLOAT_DATOR:
				return (*(float*)m_data);
			default:
				return 0;
			}
		}

		/// Converteste la char *
		operator char*()
		{
			return m_data;
		}

	protected:
		int m_type;		///< Tipul informatiei
		int m_size;		///< Dimensiunea
		char *m_data;	///< Informatia propriu-zisa

		/// Verifica daca un sir este numar
		int is_number( char * text )
		{
			if( !text ) return EMPTY_DATOR;
			int point = 0;
			int l = strlen(text);
			for(int i=0;i<l;i++)
			{
				if( !isdigit(text[i]) && text[i] != '.') 
				{
					return STRING_DATOR;
				}
				else
				{
					if( text[i] == '.' ) point++;
				}
			}
			if( !point ) return INT_DATOR;
			if( point == 1) return FLOAT_DATOR;
			return STRING_DATOR;
		}

	};	// class CDator

};	// namespace GSTD

#endif	// GARAJ_DATOR_H