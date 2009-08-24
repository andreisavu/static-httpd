
// HTTP Server utility classes
/*

Functii utile pentru server

(c) GarajCode 2005-2006 - programmed by Savu Andrei
Date : 27 februarie 2006
Last date : 27 februarie 2006

*/

#ifndef GARAJ_HTTP_SERVER_UTILITY_H
#define GARAJ_HTTP_SERVER_UTILITY_H

#include "gsocket.h"
#include "gini.h"
#include <string>

/// Garaj Network Framework
namespace GN 
{

/// HTTP Request parse
/**
	Imparte cererea HTTP in componente. Poate
	fi folosit pentru un sistem de plugin-uri 
	pentru a avea continut dinamic pe server.
  */
class HttpRequest
{
protected:
	std::string m_method;			///< Metoda de cerere GET, POST etc.
	std::string m_url;				///< URL-ul cerut de browser
	std::string m_http_version;		///< Versiunea HTTP a browser-ului
	GSTD::CIni m_ini;				///< Restul de variabile din cerere

public:

	/// Un iterator pentru lista de tag-uri din cererea HTTP
	typedef GSTD::CIni::iterator TagsIterator;

	/// Initializare
	HttpRequest() {}

	/// Initializare cu parametrii
	HttpRequest( char * request, int l ) { parse(request,l); }

	// Proceseaza cererea http
	void parse( char * http_request, int l );

	/// Returneaza url-ul din cerere
	const char * url() { return m_url.c_str(); }

	/// Returneaza metoda folosita pentru cerere
	const char * method() { return m_method.c_str(); }

	/// Returneaza versiunea de cerere
	const char * version() { return m_http_version.c_str(); }

	/// Un tag exista sau nu
	int is_set( char *var_name ) { return m_ini.is_set(var_name); }

	/// Returneaza informatia tagului respectiv
	const GSTD::CDator& operator[] ( char * tag ) { return m_ini[tag]; }

	/// Returneaza iterator inceput lista de tag-uri
	TagsIterator tags_begin() { return m_ini.begin(); }

	/// Returneaza iterator sfarsit lista de taguri
	TagsIterator tags_end() { return m_ini.end(); }
};


/// HTTP URL parsing class
/**
	O clasa pentru gestionarea URL.
	Acesta este impartit in mai multe bucati
	de forma <nume>=<valoare>. Toate sunt
	retine intr-o structura gen ini. 
*/
class HttpUrl
{
private:
	std::string m_file;			///< Fisierul cerut
	std::string m_query;		///< Query string
	std::string m_ext;			///< File extension - used for content type
	GSTD::CIni m_ini;			///< Pentru stocarea valorilor primite

	int decode( char * url );

public:

	/// Un iterator pentru lista de tag-uri din cererea HTTP
	typedef GSTD::CIni::iterator QueryIterator;
	
	/// Initializare
	HttpUrl() {}

	/// Initializare cu parametrii
	HttpUrl( const char * url, const char *base_folder ) { parse( url, base_folder ); }
	
	// Decodeaza url-ul
	int parse( const char * url, const char *base_folder );

	/// Returneaza numele fisierului
	const char *file() { return m_file.c_str(); }

	/// Returneaza extensia fisierului
	const char *ext() { return m_ext.c_str(); }

	/// Un tag exista sau nu
	int is_set( char *var_name ) { return m_ini.is_set(var_name); }

	/// Returneaza informatia tagului respectiv
	const GSTD::CDator& operator[] ( char * tag ) { return m_ini[tag]; }

	/// Returneaza iterator inceput lista de query
	QueryIterator query_begin() { return m_ini.begin(); }

	/// Returneaza iterator sfarsit lista de query
	QueryIterator query_end() { return m_ini.end(); }
};


#define HTTP_OK  200
#define HTTP_CREATED 201
#define HTTP_ACCEPTED 202
#define HTTP_NO_CONTENT 204

#define HTTP_REDIRECT 300
#define HTTP_MOVED_PERMANENTLY 301
#define HTTP_MOVED_TEMPORARILY 302
#define HTTP_NOT_MODIFIED 304

#define HTTP_BAD_REQUEST 400
#define HTTP_UNAUTHORIZED 401
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404

#define HTTP_SERVER_ERROR 500
#define HTTP_NOT_IMPLEMENTED 501
#define HTTP_BAD_GATEWAY 502
#define HTTP_SERVICE_UNAVAIBLE 503


/// HTTP Respons class
/**
	Clasa folosita pentru a retine datele din
	header-ul care va fi trimis clientului. 
	Permite o utilizare foarte usoara.
  */
class HttpResponse
{
private:
	std::string m_txt;	///< Prima linie care este variabila
	GSTD::CIni m_ini;	///< Restul informatiilor care sunt trimise sub forma <nume>: <valoare>\n
	int m_sent;			///< Raspunsul trebuie trimis o singura data 

public:

	/// Un iterator pentru lista de tag-uri din cererea HTTP
	typedef GSTD::CIni::iterator ResponseIterator;

	// Initializare
	HttpResponse():m_sent(false) {}

	/// Initializare cu parametrii
	HttpResponse( char * first_line ):m_txt( first_line ), m_sent(false) {}
	HttpResponse( int code ):m_sent(false) { set_status(code); }

	/// Trimite  header raspuns la cererea http
	void send( GN::CSocket& s );

	/// Seteaza o noua valoare
	void set( char *var, char *value ) { m_ini.set( var, value ); }
	
	/// Seteaza prima linie din header de raspuns
	void set_status( char *first_line ) { m_txt = first_line; }
	void set_status( int code );

	// Seteaza header pentru redirectare
	void redirect( char *dest );

	/// Un tag exista sau nu
	int is_set( char *var_name ) { return m_ini.is_set(var_name); }

	/// Returneaza informatia tagului respectiv
	const GSTD::CDator& operator[] ( char * tag ) { return m_ini[tag]; }

	/// Returneaza iterator inceput lista de tag-uri
	ResponseIterator query_begin() { return m_ini.begin(); }

	/// Returneaza iterator sfarsit lista de taguri
	ResponseIterator query_end() { return m_ini.end(); }
};


/// Definitia functiei pentru plugin
/**
	Pentru continut dinamic serverul se bazeaza pe plugin-uri.
	Un plungin este un DLL care exporta o functie numita HttpPlugin
	definita astfel. Aceasta functie trebuie sa trimita raspunsul
	inainte de orice altceva. Pentru a evita situatiile in care DLL-ul
	este downloadat tipul continutului trebuie setat text/html
  */
typedef int (*PLUGIN_FUNCTION)(HttpRequest& requst, HttpUrl& url, HttpResponse& response, CSocket& client );

}; // GN

#endif
