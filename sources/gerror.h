/*
  (c) GarajCode
  Garaj error handle header
  Date : 4 iulie 2005
  Last date : 19 iulie 2005
*/

/*
	Toata libraria standard garaj foloseste
	acest header pentru a semnala errori.
  */

#ifndef GARAJ_ERROR_HANDLE_H
#define GARAJ_ERROR_HANDLE_H

#define MAX_ERROR_BUFFER 100	///< Lungimea maxima a bufferului pentru erori

#include <string.h>

namespace GSTD
{

	/// Erori standard
	enum STD_ERROR
	{
		BAD_ALLOC,			///< Eroare la alocare memorie
		BUFFER_UNDERFLOW,   ///< Depasire limita inferioara buffer
		BUFFER_OVERFLOW		///< Depasire limita superioara buffer
	};		

	
	/// Clasa pentru semnalare de erori
	/**
		Pe aceasta clasa se bazeaza toata 
		libraria GarajCode pentru semnalare de erori.
		O eroare este identificata printr-un cod de
		eroare si un text scurt explicativ. Semnalarea
		erorilor in librarie se face folosind exceptii.
	*/
	class CError
	{
	public:
		/// Constructor
		CError()
		{
			m_code = 0;
			m_text[0] = 0;
		}
		CError(STD_ERROR code)
		{
			pack( code );
		}
		CError( int code, char *text)
		{
			pack( code, text );
		}

		/// Returneaza codul erorii
		int get_code()
		{
			return m_code;
		}

		/// Returneaza textul erorii
		char * get_text()
		{
			return m_text;
		}

		/// Seteaza codul erorii
		void set_code( int code )
		{
			m_code = code;
		}

		/// Seteaza textul explicativ
		void set_text( char *text )
		{
			int l = strlen(text);
			if( l > MAX_ERROR_BUFFER) l = MAX_ERROR_BUFFER;

			// avoid buffer overflow
			m_text[0] = 0;
			strncat( m_text, text, l);
		}
		
		/// Initializeaza structura cu datele transmise
		void pack( int code, char *text )
		{
			m_code = code;

			int l = strlen(text);
			if( l > MAX_ERROR_BUFFER) l = MAX_ERROR_BUFFER;

			// avoid buffer overflow
			m_text[0] = 0;
			strncat( m_text, text, l);
		}
		
		///	Seteaza clasa pentru o eroare standard
		void pack( STD_ERROR code )
		{
			m_code = code;
			switch( code )
			{
			case BAD_ALLOC:
				strcpy(m_text, "Memory allocation error");
				break;
			case BUFFER_UNDERFLOW:
				strcpy(m_text, "Buffer underflow");
				break;
			case BUFFER_OVERFLOW:
				strcpy(m_text, "Buffer overflow");
				break;
			default:
				strcpy(m_text, "Unknown error");
			}
		}

	protected:
		int m_code;								///< Codul erorii
		char m_text[ MAX_ERROR_BUFFER+1 ];		///< O scurta explicatie pentru eroare

	};	// CError
};

#endif
