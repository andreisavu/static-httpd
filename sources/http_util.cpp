
// HTTP Server utility classes
/*

Functii utile pentru server

(c) GarajCode 2005-2006 - programmed by Savu Andrei
Date : 27 februarie 2006
Last date : 27 februarie 2006

*/

#include "http_util.h"

// Garaj Network Framework
namespace GN
{

// Definire clasa pentru preprocesare cerere HTTP

/// Procesare cerere HTTP
/**
	Aceasta functie extrage din cererea HTTP metoda de cerere,
	url-ul, versiunea implementarii http si toti parametrii
	care urmeaza. r - cererea Http  l-lungimea ei
  */
void HttpRequest::parse( char * r, int l )
{
	char *p = r, *v;
	int i=0;
	
	// extrage metoda folosita - mergi pana la primul spatiu.
	while( r[i]!=' ' && i<l ) i++;
	r[i] = 0;
	m_method = p;
	i++;

	// extrage url - care nu contine spatii 
	while( r[i]==' ' && i<l ) i++;
	p = &r[i];
	while( r[i]!=' ' && i<l ) i++;
	r[i]=0;
	m_url = p;
	i++;

	// extrage versiune http - pana la primul \n
	while( r[i]==' ' && i<l ) i++;
	p = &r[i];
	while( r[i]!='\n' && i<l ) i++;
	r[i]=0;
	m_http_version = p;
	i++;

	// extrage restul parametrilor care
	// sunt trimisi sub forma <nume>: <valoare>\n
	do
	{
		if( r[i]=='\n' || r[i+1]=='\n' ) break;

		// extrage numele
		p = &r[i];
		while( r[i]!=':' && r[i]!='\n' && r[i]!=' ' && i<l ) i++;
		r[i]=0; i++;
		while( r[i]==' ' && i<l ) i++;
	
		// extrage valoarea
		v = &r[i];
		while( r[i]!='\n' && i<l ) i++;
		r[i-1] = 0;

		// creaza o variabila cu numele si valoarea gasita
		m_ini.set( p, v );
		i++;
		
	}while( 1 );
}

/// Converteste un numar din hexa in dec
inline int hex1_to_dec( char a )
{
	// daca e cifra
	if( a>='0' && a<='9' ) return a-'0';

	// daca e litera mica
	if( a>='a' && a<='f') return a-'a';

	// daca e litera mare 
	if( a>='A' && a<='F' ) return a-'A';

	// eroare - cifra necunoscuta
	return -1;
}


/// Converteste un numar hexa
/**
	Functie folosita pentru decodarea 
	url. Converteste un numar hexa de doua
	cifre intr-un numar intreg
  */
inline int hex2_to_dec( char a, char b )
{
	int c1 = hex1_to_dec(a), c2 = hex1_to_dec(b);
	if( c1==-1 || c2==-1 ) 
	{
		// eroare - a aparut un caracter necunoscut
		return -1;   
	}
	return c1*16+c2;
}


/// Decodare URL
/**
	Decodare URL - inlocuieste grupurile de trei
	litere care incep cu % cu caracterul ASCII 
	corespunzator.
  */
int HttpUrl::decode( char * url )
{
	int l = strlen(url), i=0, j=0, aux;
		
	while( i<l )
	{
		if( url[i]=='%' )
		{
			// urmatoarele doua caractere sunt un numar
			aux = hex2_to_dec( url[i+1], url[i+2] );
			if( aux == -1 ) return 0;	

			// pune caracterul corespunzator
			url[j] = (char)aux;
			i+=3;
			j++;
		}
		else 
		{
			// copiaza caracterul pur si simplu
			url[j] = url[i];
			if( url[j]=='/' ) url[j]='\\';
			i++;
			j++;
		}
	}

	url[j] = 0;

	return 1;
}	

/// Procesare url
/**
	Imparte url-ul in componente logice. 
	Nume fisier + eventuale variabile trimise prin GET
*/
int HttpUrl::parse( const char * http_url, const char * base_folder )
{
	// copiaza sirul de caractere din url
	char *url, *ext;
	if( !(url = new char[ strlen(http_url)+2 ]) ) return 0;
	strcpy( url, http_url );

	// decodeaza url-ul - caracterele care incep cu %
	if( !decode( url ) ) return 0;

	// proceseaza url
	char *p = url;
	int i=0, l=strlen(url);

	// extrage numele fisierului, pana la primul ? sau #
	// si extensia pentru a putea stabilii content-type
	ext = NULL;
	while( url[i]!='?' && url[i]!='#' && i<l ) 
	{
		if( url[i] == '.' ) ext = &url[i+1];
		i++;
	}
	url[i] = 0;

	m_file = base_folder;
	m_file += p;
	if( ext ) m_ext = ext;

	// verifica daca avem sau nu nume de fisier
	if( p[strlen(p)-1] == '\\' ) 
	{
		m_file += "index.html";
		m_ext = "html";
	}
	i++;
	
	// extrage string-ul pentru query
	p = &url[i];
	while( url[i]!='#' && i<l ) i++;
	url[i] = 0;
	m_query = p;

	// proceseaza string si extrage valorile
	// valorile sunt separate prin = si &
	l = strlen(p);
	char *r = p;
	i = 0;

	char *nume, *valoare;
	bool has_value;
	while( i<l )
	{
		// extrage numele
		nume = &p[i];
		has_value = true;
		while( p[i]!='=' && p[i]!='&' && i<l ) i++;
		if( p[i] == '&' ) 
		{
			// avem doar numele
			has_value = false;
			valoare = NULL;
		}
		p[i] = 0;
		i++;
		
		// extrage valoarea
		if( has_value )
		{
			valoare = &p[i];
			while( p[i]!='&' && i<l ) i++;
			p[i] = 0; 
			i++;
		}

		// adauga o variabila cu acest nume
		m_ini.set( nume, valoare );

	}

	// elibereaza memoria folosita
	delete[] url;

	return 1;
}

// trimite new line - \n
inline nwl( GN::CSocket& s )
{
	try
	{
		char n = '\n';
		s << n;
	}
	catch(...)
	{
		throw;
	}
}

/// Trimite header de raspuns http
/**
	Trimite header de raspuns pentru cererea 
	http. Header-ul poate contine orice.
  */
void HttpResponse::send( GN::CSocket& s )
{
	// trimite prima linie si apoi
	// fiecare variabila sub forma <nume>: <valoare>\n
	// header-ul se incheie cu un \n dublu

	if( m_sent ) return;
	
	try
	{
		s.send( m_txt.c_str(), m_txt.length() ); nwl(s);

		// MessageBox( NULL, m_txt.c_str(), "dfg", MB_OK );

		for( GSTD::CIni::iterator it=m_ini.begin(); it!=m_ini.end(); it++ )
		{
			//	MessageBox( NULL, (*it).second,(*it).first.c_str() , MB_OK );
			if( (*it).second.get_size() )
			{
				s.send( (*it).first.c_str(), (*it).first.length() );
				s.send( ": ", 2 );
				s.send( (*it).second, (*it).second.get_size() );
				nwl(s);
			}
		}

		nwl(s);

		m_sent = true;
	}
	catch(...)
	{
		throw;
	}
}

/// Set HTTP Response header
void HttpResponse::set_status( int code )
{
	switch( code )
	{
	case HTTP_OK:
		set_status( "HTTP/1.0 200 OK" );
		break;

	case HTTP_CREATED:
		set_status( "HTTP/1.0 201 Created" );
		break;

	case HTTP_ACCEPTED:
		set_status( "HTTP/1.0 202 Accepted" );
		break;

	case HTTP_NO_CONTENT:
		set_status( "HTTP/1.0 204 No Content" );
		break;
	

	case HTTP_REDIRECT:
		set_status( "HTTP/1.0 300 Multiple choice" );
		break;

	case HTTP_MOVED_PERMANENTLY:
		set_status( "HTTP/1.0 301 Moved Permanently" );
		break;

	case HTTP_MOVED_TEMPORARILY:
		set_status( "HTTP/1.0 302 Moved Temporarily" );
		break;

	case HTTP_NOT_MODIFIED:
		set_status( "HTTP/1.0 304 Not Modified" );
		break;

	case HTTP_BAD_REQUEST:
		set_status( "HTTP/1.0 400 Bad request" );
		break;

	case HTTP_UNAUTHORIZED:
		set_status( "HTTP/1.0 401 Unauthorized" );
		break;

	case HTTP_FORBIDDEN:
		set_status( "HTTP/1.0 403 Forbidden" );
		break;

	case HTTP_NOT_FOUND:
		set_status( "HTTP/1.0 404 Not Found" );
		break;

	case HTTP_SERVER_ERROR:
		set_status( "HTTP/1.0 500 Internal Server Error" );
		break;

	case HTTP_NOT_IMPLEMENTED:
		set_status( "HTTP/1.0 501 Not Implemented" );
		break;

	case HTTP_BAD_GATEWAY:
		set_status( "HTTP/1.0 502 Bad Gateway" );
		break;

	case HTTP_SERVICE_UNAVAIBLE:
		set_status( "HTTP/1.0 503 Service Unavaible" );
		break;
	}
}

/// Seteaza raspuns HTTP pentru redirectare
void HttpResponse::redirect( char * dest )
{
	set_status( HTTP_REDIRECT );
	set( "Location", dest );

}


};	// GN
