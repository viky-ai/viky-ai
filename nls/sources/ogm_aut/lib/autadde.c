/*
 *	Adding an extended string (or rupture string), based upon OgAutAdd
 *	Copyright (c) 1998-2008 Pertimm by Patrick Constant
 *	Dev : July 2008
 *	Version 1.0
*/
#include "ogm_aut.h"


/*
 * A rupture string is encoded by adding 1 to each letter
 * and putting zero as a char at the rupture. This enables
 * a sorting identical to the repository sorting.
*/

PUBLIC(int) OgAutAddRup(handle,iline,rupture,line)
void *handle; int iline,rupture; unsigned char *line;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
int il,l[DPcAutMaxBufferSize];
IFE(OgAutRupToExt(ctrl_aut,iline,rupture,line,&il,l));
IFE(AutAline2(ctrl_aut,l));
DONE;
}



/*
 * An extended string is encoded in 9 bits or more
 * so that we can 'encode' extra-characters.
 * For the moment (July 20th 2008), we use 9 bits
 * line must be finished with -1, i.e. line[iline]=(-1);
 * This enables multi-rupture strings or even multi-valued
 * rupture string. 
*/

PUBLIC(int) OgAutAddExt(handle,iline,line)
void *handle; int iline; int *line;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
IFE(OgAutCheckLine(ctrl_aut,"OgAutAddExt",iline,line));
IFE(AutAline2(ctrl_aut,line));
DONE;
}




int OgAutCheckLine(ctrl_aut,funcname,iline,line)
struct og_ctrl_aut *ctrl_aut; char *funcname;
int iline; int *line;
{
char erreur[DOgErrorSize];
int i;

if (line[iline]!=(-1)) {
  sprintf(erreur,"%s (%s): line[%d] (%d) != (-1)"
    ,funcname,ctrl_aut->name,iline,line[iline]);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

for (i=0; i<iline; i++) {
  if (line[i] >= 1<<DPcMxExtlettre) {
    sprintf(erreur,"%s (%s): line[%d] (%d) > %d"
      ,funcname,ctrl_aut->name,i,line[i],1<<DPcMxExtlettre);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
  }

DONE;
}





int OgAutRupToExt(ctrl_aut,iline,rupture,line,pil,l)
struct og_ctrl_aut *ctrl_aut; 
int iline,rupture; unsigned char *line;
int *pil, *l;
{
int i,j;

if (rupture < 0) {
  for (i=j=0; i<iline; i++) {
    l[j++]=line[i]+1;
    }
  l[j]=(-1);
  }
else {
  for (i=j=0; i<rupture; i++) {
    l[j++]=line[i]+1;
    }
  l[j++]=0;
  for (; i<iline; i++) {
   l[j++]=line[i]+1;
    }
  l[j]=(-1);
  }
*pil=j;  
DONE;
}



