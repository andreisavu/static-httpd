/*
(c) GarajCode 2005
General utility functions
Date : 23 noiembrie 2005
Last date : 23 februarie 2005
*/

#ifndef GARAJ_UTILITY_H
#define GARAJ_UTILITY_H

#include <windows.h>

/// Garaj Standard Namespace
namespace GSTD
{

/// Gestionare evenimente windows in consola
/**
  Clasa captureaza evenimetele windows care cer inchiderea
  aplicatiei care foloseste o consola. Pentru a face acest
  lucru instaleaza un nou control handler pentru consola.
  */
class ConsoleCtrl
{
public:
	ConsoleCtrl();
	~ConsoleCtrl();
	void set();
	void unset();
	bool exit_request();

protected:
	friend bool CtrlHandler( DWORD type );
	static bool m_exit;
	static int m_refc;

};	

};	// namespace GSTD

#endif	// GARAJ_UTILITY_H