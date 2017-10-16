/*
 *	Gestion des erreurs.
 *	Copyright (c) 1996	Patrick Constant
 *	Dev : Janvier 1996
 *	Version 1.0
*/
#include <lpcmcr.h>
#include <lpcosys.h>
#include <lpcerr.h>


static int PcErrMaxh(void);
static int PcErrHpos(int, int);
static int PcErrNbh(int);
static void PcErrTrans(int);
static int PcErrCpy(int, char *);



/*
 *	Tableau de taille fixe dont les cha\^ines sont de taille 
 *	fixe (pour \'eviter les probl\`emes concurrents de m\'emoire) 
 *	DPcSzErr est d\'efini dans lpcmem.h
*/

#define	DPcNbErr	10


struct SPcMemErr {
  int nombre;
  int handle[DPcNbErr];
  char erreur[DPcNbErr][DPcSzErr];
  };

static struct SPcMemErr PcMemErr={0};



/*
 *	Fonction d'ajout d'un message d'erreur dans la pile 
 *	Le but du jeux est de garder au moins le premier et 
 *	le dernier message sur une liste de messages ayant le m\^eme handle.
 *	Si ce handle poss\`ede au moins trois messages, on 
 *	enl\`eve celui du milieu pour faire de la place pour le dernier.
 *	Sinon, il s'agit de retrouver le handle qui contient le plus 
 *	de messages (au moins trois) et d'enlever celui du milieu.
 *	Si tous les handles ont moins de trois \'el\'ements,
 *	on met le message d'office \`a la fin.
*/
 

PUBLIC(int) PcErr(int handle, char *erreur)
{
int nbh,pos,maxh;
int position=DPcNbErr-1;
if (PcMemErr.nombre>=DPcNbErr) {
  nbh=PcErrNbh(handle);
  if (nbh>=3) {
    pos=PcErrHpos(handle,nbh/2);
    PcErrTrans(pos);
    }
  else {
    maxh=PcErrMaxh();
    nbh=PcErrNbh(maxh);
    if (nbh>=3) {
      pos=PcErrHpos(maxh,nbh/2);
      PcErrTrans(pos);
      }
    }
  }
else position=PcMemErr.nombre++;
PcMemErr.handle[position]=handle;
PcErrCpy(position,erreur);
DONE;
}



static int PcErrMaxh(void)
{
int i,j,max,maxh,nh;
int th[DPcNbErr], tm[DPcNbErr];
memset(th,0,sizeof(int)*DPcNbErr);
memset(tm,0,sizeof(int)*DPcNbErr);
for (nh=0,i=0; i<DPcNbErr; i++) {
  for (j=0; j<nh; j++)
    if (th[j]==PcMemErr.handle[i]) tm[j]++;
  if (j==nh) { th[j]=PcMemErr.handle[i]; tm[j]=1; nh++; }
  }
maxh=th[0]; max=tm[0];
for (j=0; j<nh; j++) 
  if (max<tm[j]) { max=tm[j]; maxh=th[j]; }
return(maxh);
}



static int PcErrHpos(int handle, int hpos)
{
int i,nbh;
for (nbh=0,i=0; i<DPcNbErr; i++)
  if (PcMemErr.handle[i]==handle) {
    if (nbh==hpos) return(i); nbh++;
    }
return(DPcNbErr-1);
}



static int PcErrNbh(int handle)
{
int i,nbh;
for (nbh=0,i=0; i<DPcNbErr; i++)
  if (PcMemErr.handle[i]==handle) nbh++;
return(nbh);
}




static void PcErrTrans(int pos)
{
int i;
for (i=pos+1; i<DPcNbErr; i++) {
  memcpy(PcMemErr.erreur[i-1],PcMemErr.erreur[i],DPcSzErr);
  PcMemErr.handle[i-1]=PcMemErr.handle[i];
  }
}




static int PcErrCpy(int position, char *erreur)
{
int len=strlen(erreur);
if (len>DPcSzErr-1) len=DPcSzErr-1;
memcpy(PcMemErr.erreur[position],erreur,len);
PcMemErr.erreur[position][len]=0;
DONE;
}




/*
 *	Recopie dans 'erreur' (qui doit \^etre allou\'e)
 *	le dernier message d'erreur s'il existe. Renvoit 1 dans ce cas.
 *	S'il n'y a pas d'erreur, renvoit 0 et met la chaine "" dans erreur.
 *	Ce message d'erreur doit correspondre au handle, cependant
 *	certains message d'erreur se font avant l'attribution
 *	d'un handle, ou n'ont pas acc\`es au handle, dans 
 *	ce cas PcErrLast r\'ecup\`ere aussi ce message.
*/

PUBLIC(int) PcErrLast(int handle, char *erreur)
{
int i;
/* Par construction PcMemErr.nombre ne pas être > DPcNbErr
 * mais comme on n'est pas thread safe, cela peut arriver.
 * C'est pour cela qu'on fait un test ici. */
if (PcMemErr.nombre>DPcNbErr) {
  PcMemErr.nombre=DPcNbErr;
  }
for (i=PcMemErr.nombre-1; i>=0; i--) {
  if (PcMemErr.handle[i]==handle || PcMemErr.handle[i]==(-1)) break;
  }
if (i<0) { erreur[0]=0; return(0); }
memcpy(erreur,PcMemErr.erreur[i],DPcSzErr);
memset(PcMemErr.erreur[PcMemErr.nombre],0,sizeof(char)*DPcSzErr);
PcMemErr.nombre--;
PcErrTrans(i);
return(1);
}




PUBLIC(int) PcErrDiag(int *handle, char *erreur)
{
if (PcMemErr.nombre<=0) return(0);
/* Par construction PcMemErr.nombre ne pas être > DPcNbErr
 * mais comme on n'est pas thread safe, cela peut arriver.
 * C'est pour cela qu'on fait un test ici. */
if (PcMemErr.nombre>DPcNbErr) {
  PcMemErr.nombre=DPcNbErr;
  }
*handle=PcMemErr.handle[PcMemErr.nombre-1];
memcpy(erreur,PcMemErr.erreur[PcMemErr.nombre-1],DPcSzErr);
memset(PcMemErr.erreur[PcMemErr.nombre-1],0,sizeof(char)*DPcSzErr);
PcMemErr.nombre--;
return(1);
}






