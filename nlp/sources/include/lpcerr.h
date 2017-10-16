/*
 *	Fichier g\'en\'eral de d\'efinition pour la gestion d'erreur.
 *	Copyright (c) 1996,97	Patrick Constant
 *	Dev : Janvier,Mars,Septembre 1996, Mars 1997, Octobre 1999
 *	Version 1.5
*/

#ifndef _LOGERRALIVE_

#include <lpcmcr.h>
#include <lpcosys.h>


/**	Taille maximale du message d'erreur **/
#define DPcSzErr	256
/*	Taille maximale du message d'erreur systeme
 *  Il reste 50 octets pour rajouter des informations
*/
#define DPcSzSysErr	(DPcSzErr-80)


DEFPUBLIC(int) PcErr(pr_(int) pr(char *));
DEFPUBLIC(int) PcErrLast(pr_(int) pr(char *));
DEFPUBLIC(int) PcErrDiag(pr_(int *) pr(char *));



#define _LOGERRALIVE_
#endif
