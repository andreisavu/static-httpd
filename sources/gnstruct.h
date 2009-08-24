// GarajCode
// General network structures
// date : 12 august 2005
// last date : 16 august 2005
// change log:
//	16 august 2005 - modificata dimensiune text

// ATENTIE !!!!
/*
	Modificari facute aici pot provoca probleme
	de comunicare. Dupa ce se face o modificarea 
	in  ceea ce priveste aceasta structura 
	pentru a pastra compatibilitatea programele 
	trebuie sa fie recompilate.
  */

#ifndef GARAJ_NETWORK_GENERAL_USE_STRUCTURES_H
#define GARAJ_NETWORK_GENERAL_USE_STRUCTURES_H

#define GN_MSG_PARAM_NR 2		///< Numarul maxim de parametrii din mesaj
#define GN_MAX_MSG_TEXT 128		///< Lungimea maxima a textului din mesaj

/// Garaj Network Namespace
namespace GN
{
	/// O structura statica pentru mesaje in retea
	/**
		O structura foarte simpla alocata statica pentru a 
		trimite mesaje in retea. Datorita faptului ca este 
		alocata static transmiterea se face foarte usor.
	*/
	struct NetMessage
	{
		int message;					///< Codul mesajului
		int param[ GN_MSG_PARAM_NR ];	///< Parametrii numere intregi
		char text[ GN_MAX_MSG_TEXT ];	///< Un text
	};
	
};	// namespace GN

#endif	// GARAJ_NETWORK_GENERAL_USE_STRUCTURES_H