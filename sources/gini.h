/*
  (c) GarajCode
  Ini file preprocesor
  Date : 16 iulie 2005
  Last date : 25 februarie 2006
*/
// Change log:
//	17 august 2005 - marite bufferele pentru parsing
//  23 februarie 2006 - verificare existenta fisier - comentarii doxygen

#ifndef GARAJ_INI_PREPROCESOR
#define GARAJ_INI_PREPROCESOR

// dezactiveaza avertizarile STL
#pragma warning(disable:4786)
#pragma warning(disable:4786)

#include "gdator.h"

#include <fstream.h>

#include <string>
#include <map>

#define MAX_LINE_LENGTH 2056						///< Lungimea maxima a unei linii din fisierul ini
#define MAX_VAR_NAME_LENGTH 128						///< Lungimea maxima a unei variabile din ini
#define MAX_DATA_LENGTH ( MAX_LINE_LENGTH - MAX_VAR_NAME_LENGTH - 5 )	///< Lungimea maxima a valorii unei variabile

/// Garaj Standard Namespace
namespace GSTD
{
	/// Informatia este retinuta in perechi de forma string si informatie
	typedef std::pair<std::string, CDator> INI_DATA;

	/// Procesor pentru fisiere ini
	/**
	Aceasta clasa implementeaza un procesor de fisiere ini.
	In fisier informatiile sunt de forma <nume>=<valoare>.
	Fiecare expresie de genul asta trebuie sa fie singura pe 
	un rand. Sirurile de caractere care contin spate trebuie
	puse intre ghilimele.
	*/
	class CIni
	{
	public:
		CIni() 
		{
		}

		CIni( const CIni & ini )
		{
			m_data = ini.m_data;
		}

		CIni & operator = ( const CIni & ini )
		{
			m_data = ini.m_data;
			return *this;
		}

		~CIni()
		{
			clear();
		}

		/// Incarca informatiile din fisierul de configurare
		int load( char * file )
		{
			// elibereaza datele actuale
			clear();

			// verifica daca fisierul exista 
			FILE *f = fopen( file, "r" );
			if( !f ) return 0;
			fclose(f);

			// deschide fisier
			fstream fin(file, ios::in);

			// citeste linie
			char buffer[ MAX_LINE_LENGTH ];
			char var_name[ MAX_VAR_NAME_LENGTH ];
			char data[ MAX_DATA_LENGTH ];
			while( fin.getline( buffer, MAX_LINE_LENGTH ) )
			{
				// verifica daca avem comentariu
				if( is_comment(buffer) ) continue;

				// imparte linia in doua blocuri fata de =
				if( !split(buffer,var_name,data) ) continue;

				// adauga in lista noua variabila
				if( !m_data.count(var_name) )
				{
					INI_DATA new_data;

					new_data.first = var_name;
					new_data.second.convert(data);

					m_data.insert(new_data);
				}
				else
				{
					m_data[ var_name ].convert(data);
				}
			}
			return 1;
		}
		
		/// Elibereaza memoria alocata pentru date
		void clear()
		{
			m_data.clear();
		}

		/// Seteaza o anumita variabila cu un numar intreg
		void set( char* var_name, int value )
		{
			if( m_data.count( var_name ) )
			{
				m_data[ var_name ].set( value );
			}
			else
			{
				INI_DATA new_data;
				new_data.first = var_name;
				new_data.second.set( value );
				m_data.insert( new_data );
			}
		}

		/// Seteaza o variabila cu un numar real
		void set( char* var_name, float value )
		{
			if( m_data.count( var_name ) )
			{
				m_data[ var_name ].set( value );
			}
			else
			{
				INI_DATA new_data;
				new_data.first = var_name;
				new_data.second.set( value );
				m_data.insert( new_data );
			}
		}

		/// Seteaza o variabila cu un sir terminat cu zero
		void set( char* var_name, char *value )
		{
			if( m_data.count( var_name ) )
			{
				m_data[ var_name ].convert( value );
			}
			else
			{
				INI_DATA new_data;
				new_data.first = var_name;
				new_data.second.convert( value );
				m_data.insert( new_data );
			}
		}

		/// Verifica daca o variabila exista
		int is_set( char* var_name )
		{
			return m_data.count( var_name );
		}

		/// Returneaza o variabila
		CDator & get( char *var_name )
		{
			if( is_set(var_name) ) return m_data[ var_name ];
			return m_empty_dator;
		}

		/// Acceseaza variabilele
		CDator & operator [] ( char *var )
		{
			return get(var);
		}

		/// Iterator pentru datele retinute
		typedef std::map< std::string, CDator >::iterator iterator;

		/// Returneaza un iterator la inceputul dictionarului
		iterator begin() { return m_data.begin(); }

		/// Returneaza un iterator la sfarsitul dictionarului
		iterator end() { return m_data.end(); }

	protected:
		std::map< std::string, CDator > m_data;		///< Dictionarul in care sunt stocate informatiile
		CDator m_empty_dator;						///< O variabila goala

		/// Verifica daca sirul este un comentariu. Orice comentariu incepe cu #
		bool is_comment( char *buffer )
		{
			int l = strlen(buffer);
			int i=0;
			// skip initial white spaces
			while( buffer[i] == ' ' || buffer[i] == '\t') i++;
			// check for comment
			if( buffer[i] == '#' ) return true;
			return false;
		}

		/// Sare peste spatii albe
		void skip_white_space( int &i, char *buffer, int l )
		{
			while( (buffer[i] == ' ' || buffer[i] == '\t') && i<l ) i++;
		}

		/// Imparte sirul in doua subsiruri: nume variabila si informatie
		int split( char *buffer, char * var, char *data)
		{
			int k;
			int l = strlen(buffer);
			int i = 0;

			// sare peste spatiile initiale
			skip_white_space( i, buffer, l );

			// copiaza numele variabilei
			k = 0;
			while( buffer[i] != ' ' && buffer[i] != '\t' && buffer[i] != '=' && i < l)
			{
				var[ k ] = buffer[i];
				k++;
				i++;
			}
			var[k] = 0;
			
			// daca avem un sir vid atunci avem o problema
			if( !k ) return 0;

			if( buffer[i] == '=' ) i++;

			// sari peste spatii albe
			skip_white_space( i, buffer, l);

			// vezi daca am ajuns la egal
			if( buffer[i] == '=' ) i++;

			// evita spatii albe
			skip_white_space( i, buffer, l);

			// verifica daca avem un string
			if( buffer[i] == '\"' )
			{
				// copiaza tot pana la urmatoarea ghilimea
				i++;
				k = 0;
				while( buffer[i] != '\"' && i < l)
				{
					data[k] = buffer[i];
					k++;
					i++;
				}
				data[k] = 0;
				return 1;
			}

			// copiaza pana la primul spatiu liber sau comentariu 
			k = 0;
			while( buffer[i] != ' ' && buffer[i] != '#' && buffer[i] != '\t' && i < l)
			{
				data[k] = buffer[i];
				k++;
				i++;
			}
			data[k] = 0;
			if( !k ) return 0;
			return 1;
		}

	};

};

#endif

// (c) GarajCode 2005 - programmed by Savu Andrei

