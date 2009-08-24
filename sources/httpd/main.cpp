// GarajCode
// http daemon
// date : 12 august 2005
// last date : 27 februarie 2006
// change log :
//	12 august 2005 - start proiect
//  27 februarie 2006 - multe lucruri noi 

// dezactiveaza avertizarile pentru STL
#pragma warning(disable:4786)
#pragma warning(disable:4786)

#include "..\\http_server.h"
#include "..\\gini.h"
#include "..\\gutil.h"

#include <iostream.h>

GSTD::CIni g_ini;		///< Ini file procesor

/// Functia main
int main()
{
	// incarca fisierul de configurare
	g_ini.load("http.ini");

	// seteaza valori standard pentru variabilele nedefinite
	if( !g_ini.is_set("max_http_connections") ) g_ini.set("max_http_connections", 256 );
	if( !g_ini.is_set("http_port") ) g_ini.set("http_port", HTTP_PORT );
	if( !g_ini.is_set("http_mcsc") ) g_ini.set("http_mcsc", 1 );
	if( !g_ini.is_set("http_ubip") ) g_ini.set("http_ubip", 0 );
	if( !g_ini.is_set("http_utip") ) g_ini.set("http_utip", 0 );

	// Console exit event handler
	GSTD::ConsoleCtrl con;
	
	try
	{
		GN::CWinsock winsock;
		GN::CServer http_server;
		GN::ServerCreationData sd;

		sd.max_client_number = (int)g_ini["max_http_connections"];
		sd.mcsc = (int)g_ini["http_mcsc"];
		sd.port = (int)g_ini["http_port"];
		sd.server_event_handler = GN::HTTPSEventHandler;
		sd.server_procedure = GN::HTTPServer;
		sd.ubip = (int)g_ini["http_ubip"];
		sd.utip = (int)g_ini["http_utip"];

		if( sd.utip && (char*)g_ini["http_tiplist"] ) http_server.add_tiplist( (char*)g_ini["http_tiplist"] );
		if( sd.utip && (char*)g_ini["http_biplist"] ) http_server.add_bannedlist( (char*)g_ini["http_biplist"] );
		
		// creaza serverul
		http_server.create( sd );

		cout << "* Http server started" << endl;
		cout << "* Port : " << (int)g_ini["http_port"] << endl;
		
		while( !con.exit_request() ) Sleep( 500 );
	}
	catch( GSTD::CError error )
	{
		MessageBox( NULL, error.get_text(), "Error", MB_OK);
	}
	return 0;
}


