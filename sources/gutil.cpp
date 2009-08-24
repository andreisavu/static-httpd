/*
(c) GarajCode 2005
General utility functions
Date : 23 noiembrie 2005
Last date : 23 februarie 2006
*/

#include "gutil.h"

/// Garaj Standard Namespace
namespace GSTD
{

// Utility class for handling console Ctrl events
bool ConsoleCtrl::m_exit = false;		///< Este true daca apare o cere de iesire din aplicatie
int ConsoleCtrl::m_refc = 0;			///< Numarul de instante ale clasei

/// Initializare 
ConsoleCtrl::ConsoleCtrl() 
{
	set();
}

/// Eliberare resurse
ConsoleCtrl::~ConsoleCtrl()
{
	unset();
}

/// Seteaza noul control handler pentru consola 
void ConsoleCtrl::set()
{
	if( !m_refc ) SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, true );
	m_refc++;
}

/// Reseteaza control handler-ul pentru consola
void ConsoleCtrl::unset()
{
	m_refc--;
	if( !m_refc ) SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, false );
}

/// Noua functie pentru gestionare evenimente din consola
bool CtrlHandler( DWORD type )
{
	switch( type )
	{
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		ConsoleCtrl::m_exit = true;
		return true;
	}
	return false;
}

/// returneaza true daca utilizatorul a incercat sa inchida consola
bool ConsoleCtrl::exit_request()
{
	return m_exit;
}


}; // namespace GSTD



