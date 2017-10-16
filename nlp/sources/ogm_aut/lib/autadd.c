/*
 *	Ajout d'une cha\^ine de caract\`eres dans l'automate.
 *	Copyright (c) 1997-2006 Pertimm by Patrick Constant
 *	Dev : F\'evrier 1997, March 2006
 *	Version 1.1
*/
#include "ogm_aut.h"


/*
 *	Ajout d'une cha\^ine de caract\`eres dans l'automate.
 *	Attention, il n'est pas possible de supprimer des lignes de
 *	description dans le cas o\`u l'automate est minimisé.
 *	Cependant, cette vérification n'est pas effectuée.
*/

PUBLIC(int) OgAutAdd(handle,iline,line)
void *handle; int iline; unsigned char *line;
{
int out[DPcAutMaxBufferSize];
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
IF(AutTranslateBuffer2(iline,line,out)) DONE;
IFE(AutAline2(ctrl_aut,out));
DONE;
}


