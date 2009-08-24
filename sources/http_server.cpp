
// http_server.cpp - Very Basic HTTP Server
/*

(c) GarajCode 2005-2006 - programmed by Savu Andrei
Date : 12 august 2005
Last date : 27 februarie 2006

Change log :
17 august 2005 - prima versiune
16 februarie 2006 - portat pe noua librarie
20 februarie 2006 - url encoding
25 februarie 2006 - better request processing
27 februarie 2006 - mime types - dll plugins

*/

/// Include files
#include "http_server.h"

#include "gini.h"
#include "glog.h"
#include <stdlib.h>

#include <string>

#define STD_404_PAGE "/err/404.html"  ///< Page not found
#define STD_501_PAGE "/err/501.html"  ///< Not implemented
#define STD_400_PAGE "/err/400.html"	 ///< Bad request

/// Garaj Network Framework
namespace GN
{

/// HTTP Server log file
/**
	This file is used by the HTTP server 
	to log activities. Here are recorded 
	any kind of operations made by the 
	server. It provides more information
	for debuging in case of a crash.
  */
GSTD::CLog g_httplog("Logs\\http.log");

/// Lungimea maxim a bufferului pentru cerere
#define MAX_REQUEST_RECEIVE_BUFFER 1024  


// HTTP SERVER FUNCTIONS

/// HTTP Server Event Handler
/**
  This function handles all HTTP server internal
  events. It logs error messages.
  */
bool HTTPSEventHandler( UINT msg, GSTD::CError *error, GN::CSocket * sock )
{
	switch( msg )
	{
	case GN::SM_CLIENT_CONNECTED:
		g_httplog.message("Client connected ( %s )",sock->get_ip());
		break;

	case GN::SM_CLIENT_DISCONNECTED:
		g_httplog.message("Client disconnected ( %s )",sock->get_ip());
		break;

	case GN::SM_CLIENT_ERROR:
		g_httplog.error("%s ( %s )",error->get_text(),sock->get_ip());
		break;

	case GN::SM_MCSC_ERROR:
		g_httplog.important("MCSC violation from %s",sock->get_ip());
		break;

	case GN::SM_TIP_ERROR:
		g_httplog.important("Connection from untrusted ip rejected ( %s )",sock->get_ip());
		break;

	case GN::SM_BIP_ERROR:
		g_httplog.important("Connection from banned ip rejected ( %s )",sock->get_ip());
		break;

	case GN::SM_CLIENT_LIMIT_REACHED:
		g_httplog.message("Server full, client ignored ( %s )",sock->get_ip());
		break;

	case GN::SM_CREATE:
		g_httplog.message("Server started");
		break;

	case GN::SM_CLOSE:
		g_httplog.message("Server closed");
	}

	return true;
}

/// Verifica daca cererea HTTP nu contine erori sau campuri lipsa
/**	
	Verifica daca cererea primita este corecta. Serverul 
	implementeaza doar metoda GET si are nevoie de URL.
  */
void check_for_errors( GN::CSocket& s, HttpRequest& r )
{
	// verifica corectitudine
	if( strcmp( r.method(), "GET" ) )
	{
		HttpResponse r;
		r.redirect( STD_501_PAGE );
		r.send(s);
	}
	else if( strlen( r.url() )==0 ) 
	{	
		HttpResponse r;
		r.redirect( STD_400_PAGE );
		r.send(s);
	}
	else return;	// e ok

	// a aparut o eroare la procesare cerere
	GSTD::CError error( 0, "Request parsing error" );
	throw error;
}

/// Returneaza tipul continutului returnat
/**
	Functia functioneaza pornind de la 
	extensia fisierului cerut
  */
void get_content_type( const char *ext, char *r )
{
	/// FISIERE TEXT

	// fisiere cu continut html
	if( !strcmp(ext, "html") || !strcmp(ext,"htm") )  
	{
		strcpy( r, "text/html" );
		return;
	}

	// fisiere text
	if( !strcmp(ext, "txt") || !strcmp(ext,"c") || !strcmp(ext,"c++") 
		|| !strcmp(ext,"cpp") || !strcmp(ext,"h") || !strcmp(ext, "hpp")
		|| !strcmp(ext,"pl") || !strcmp(ext,"cc") ) 
	{
		strcpy( r, "text/plain" );
		return;
	}

	// fisier cu stiluri
	if( !strcmp(ext, "css") ) 
	{
		strcpy( r, "text/css" );
		return;
	}

	// IMAGINI

	// gif
	if( !strcmp(ext, "gif") )
	{
		strcpy( r, "image/gif" );
		return;
	}

	// png
	if( !strcmp(ext, "png") )
	{
		strcpy( r, "image/x-png" );
		return;
	}

	// jpeg
	if( !strcmp(ext, "jpeg") || !strcmp(ext,"jpg") || !strcmp(ext, "jpe") )
	{
		strcpy( r, "image/jpeg" );
		return;
	}

	// tiff
	if( !strcmp(ext, "tiff") || !strcmp(ext, "tif") )
	{
		strcpy( r, "image/tiff" );
		return;
	}

	// bmp
	if( !strcmp(ext, "bmp") ) 
	{
		strcpy( r, "image/x-ms-bitmap" );
		return;
	}

	// AUDIO FILES

	// au snd
	if( !strcmp(ext, "au") || !strcmp(ext, "snd") )
	{
		strcpy( r, "audio/basic" );
		return;
	}

	// aif aiff aifc 
	if( !strcmp(ext, "aif") || !strcmp(ext, "aiff") || !strcmp(ext, "aifc") )
	{
		strcpy( r, "audio/x-aiff" );
		return;
	}

	// wav
	if( !strcmp(ext, "wav") ) 
	{
		strcpy( r, "audio/x-wav" );
		return;
	}

	// TODO: adauga si altele  http://www.utoronto.ca/webdocs/HTMLdocs/Book/Book-3ed/appb/mimetype.html#text

	// PENTRU APLICATII 

	// rtf
	if( !strcmp(ext, "rtf") )
	{
		strcpy( r, "application/rtf" );
		return;
	}

	// pdf
	if( !strcmp(ext, "pdf") )
	{
		strcpy( r, "application/x-pdf" );
		return;		
	}

	// tar
	if( !strcmp(ext, "tar") )
	{
		strcpy( r, "application/x-tar" );
		return;
	}
	
	// zip
	if( !strcmp(ext, "zip") )
	{
		strcpy( r, "application/zip" );
		return;
	}

	// exe
	if( !strcmp(ext, "exe") )
	{
		strcpy( r, "application/octet-stream" );
		return;
	}

	// js ls mocha
	if( !strcmp(ext, "js") || !strcmp(ext, "ls") || !strcmp(ext, "mocha") )
	{
		strcpy( r, "text/javascript" );
		return;
	}

	// ppz  -  power point presentation
	if( !strcmp(ext, "ppz") )
	{
		strcpy( r, "application/mspowerpoint" );
		return;
	}
}

// dump request
void dump_request( HttpRequest& req, HttpUrl& url, CSocket& s )
{
	s.printf( "<br><br><table border width=650>" );
	s.printf( "<tr><td>%s</td><td>%s</td></tr>", req.method(), url.file() );
	for( HttpRequest::TagsIterator it=req.tags_begin();
		it != req.tags_end(); it++ )
		{
			s.printf( "<tr><td><b>%s</b></td>", (*it).first.c_str() );		 
			if( !(*it).second.empty() ) s.printf( "<td>%s</td></tr>", (char*)(*it).second );
		}
	s.printf("</table>" );
	// informatiile din url
	s.printf( "<table border width=650>" );
	for( HttpUrl::QueryIterator itr=url.query_begin();
		itr != url.query_end(); itr++ )
		{
			s.printf( "<tr><td><b>%s</b></td>", (*itr).first.c_str() );		 
			if( !(*itr).second.empty() ) s.printf( "<td>%s</td></tr>", (char*)(*itr).second );
		}
	s.printf("</table>" );
}


/// HTTP Server Client handling function
/**

  This is the main function from the HTTP 
  server. Every new connection from a client
  is handled here. Here the request from the
  client is handled.
  */
void HTTPServer( GN::ClientList&, GN::CSocket & s )
{
	try
	{
		// buffer pentru cererea HTTP
		char buffer[ MAX_REQUEST_RECEIVE_BUFFER+2 ];

		HttpRequest	req;		// Clasa pentru procesare cerere HTTP
		HttpUrl url;			// Clasa pentru procesare URL
		HttpResponse r;			// Clasa pentru raspunsul HTTP
		
		// receptioneaza cerere
		int ret = s.receive( buffer, MAX_REQUEST_RECEIVE_BUFFER );
		buffer[ret]=0;

		// proceseaza cererea primita
		req.parse( buffer, ret );
		check_for_errors( s, req );

		// parse url
		url.parse( req.url(), WWW_FOLDER );
				
		// Verifica daca exista fisierul
		FILE * file = fopen( url.file(), "r");
		if( !file  )
		{
			r.redirect( STD_404_PAGE );
			r.send(s);

			GSTD::CError error( 0, "File not found");
			throw error;
		}
		fclose(file);
		
		// Trimite raspunsul ok - fisierul exista si continutul va fi trimis
		r.set_status( HTTP_OK );
		
		r.set("Server", "GCWS 1.0");
		r.set("Connection","close");
	
		// incarca DLL
		
		// seteaza tipul continutului
		get_content_type( url.ext(), buffer );
		r.set("Content-type", buffer );
		
		// trimite raspunsul cu toate campurile setate corespunzator
		r.send(s);

		// send file - one buffer at time
		file = fopen( url.file(), "rb" );
		int nb;
		do
		{
			nb = fread( buffer, 1, MAX_REQUEST_RECEIVE_BUFFER, file );
			if( nb ) s.send( buffer, nb );
		}while( nb );
		fclose( file );

		// if( url.is_set("debug") ) dump_request( req, url, s );

	}
	catch( GSTD::CError error )
	{
		throw error;
	}
}

// (c) GarajCode 2005 - programmed by Savu Andrei

};