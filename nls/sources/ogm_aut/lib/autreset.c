/*
 *	Vidange compl\`ete de l'automate dynamique.
 *	Copyright (c) 1997-2006 Pertimm by Patrick Constant
 *	Dev : Mars 1997, March 2006
 *	Version 1.1
*/

#include "ogm_aut.h"



/*
 *	Vidange compl\`ete de l'automate dynamique.
 *	Il suffit de remettre le rideau \`a z\'ero.
*/

PUBLIC(int) OgAutReset(handle)
void *handle;
{
struct og_ctrl_aut *ctrl_aut;
IFn(handle) DONE;
ctrl_aut = (struct og_ctrl_aut *)handle;
ctrl_aut->StateUsed=0;
DONE;
}



