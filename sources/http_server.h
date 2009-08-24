
// HTTP Server - Very Basic HTTP Server
/*
  
(c) GarajCode 2005-2006 - programmed by Savu Andrei
Date : 12 august 2005
Last date : 25 februarie 2006

Change log :
12 august 2005 - prima versiune
23 august 2005 - finisari
16 februarie 2006 - portat pe noua librarie
    			 -structurare fisiere intr-un folder
25 februarie 2006 - better request processing
*/

#ifndef GARAJ_HTTP_SERVER_H
#define GARAJ_HTTP_SERVER_H

#include "gnet.h"		///< Garaj Network framework
#include "gini.h"
#include "http_util.h"

#include <string>

#define HTTP_PORT 80	     ///< Standard HTTP port
#define WWW_FOLDER "www"	 ///< Web files root folder

/// Garaj Network Framework
namespace GN
{

// http server event handler
bool HTTPSEventHandler( UINT msg,
						GSTD::CError *error,
						GN::CSocket * sock );

// http server function
void HTTPServer( GN::ClientList&,
				 GN::CSocket & client );

};

#endif