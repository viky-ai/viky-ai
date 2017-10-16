/*
 *	Conversion from an occ structure to a string and vice-versa.
 *	Copyright (c) 2000	Ogmios by Patrick Constant
 *	Dev : May 2000
 *	Version 1.0
*/
#include <loggen.h>
#include <logocc.h>





/*
 *  Writes struct 'occ' with type 'occ_type' to buffer *s of length 'is'
 *  At the end, *p points to the end of the written occurrence.
 *  Returns length of written string and -1 on error.
*/

PUBLIC(int) OgOccToStr(int occ_type, struct og_occ *occ, int is, unsigned char *s)
{
int i,is_postpos;
int encoded_occ_type;
char erreur[DPcSzErr];
unsigned char *p = s;
int nbnum;

/*  We make sure that 'is' is big enough prior to filling 's'
 *  because otherwise the testing is not very efficient.
 *  There is a maximum of 5 numbers of size max 5 plus
 *  the postpos numbers.
*/
nbnum = (5+1+occ->ipostpos)*5;
if (is < nbnum*5) {
  DPcSprintf(erreur,"OgOccToStr: is %d < min %d", is, nbnum*5);
  PcErr(-1,erreur); DPcErr;
  }

is_postpos = 0;
encoded_occ_type = occ_type;
if (occ->ipostpos > 0) {
  if (occ->ipostpos >= DOgMaxPostpos) {
    DPcSprintf(erreur,"OgOccToStr: occ->ipostpos (%d) >= %d", occ->ipostpos, DOgMaxPostpos);
    PcErr(-1,erreur); DPcErr;
    }
  encoded_occ_type |= DOgOccMore;
  is_postpos = 1;
  }

OggNout(encoded_occ_type,&p);

switch(occ_type) {
  case DOgOccPos:
    OggNout(occ->position,&p);
    break;
  case DOgOccPar: 
    OggNout(occ->parag_nb,&p);
    break;
    case DOgOccParPos: 
    OggNout(occ->parag_nb,&p);
    OggNout(occ->position,&p);
    break;
  case DOgOccParWorPos: 
    OggNout(occ->parag_nb,&p);
    OggNout(occ->word_nb_in_parag,&p);
    OggNout(occ->position,&p);
    break;
  case DOgOccParSenPos: 
    OggNout(occ->parag_nb,&p);
    OggNout(occ->sentence_nb,&p);
    OggNout(occ->position,&p);
    break;
  case DOgOccParSenWorPos: 
    OggNout(occ->parag_nb,&p);
    OggNout(occ->sentence_nb,&p);
    OggNout(occ->word_nb_in_sentence,&p);
    OggNout(occ->position,&p);
    break;
  default:
    DPcSprintf(erreur,"OgOccToStr: unknown occ type %d",occ_type);
    PcErr(-1,erreur); DPcErr;
  }

if (is_postpos) {
  OggNout(occ->ipostpos,&p);
  for (i=0; i<occ->ipostpos; i++) {
    OggNout(occ->postpos[i],&p);
    }
  }

return(p-s);
}






/*
 *  Reads a string 's' with a max length 'is' and puts the result 
 *  in type pointed to by 'pocc_type' and in struct 'occ'.
 *  Returns length of read string and -1 on error.
*/

PUBLIC(int) OgStrToOcc(int is, unsigned char *s, int *pocc_type, struct og_occ *occ)
{
int i,is_postpos;
char erreur[DPcSzErr];
unsigned char *p = s;

memset(occ, 0, sizeof(struct og_occ));
occ->no_occ = 1;

IFE(*pocc_type=OggNin4(&p)); 

is_postpos = 0;
if (*pocc_type & DOgOccMore) {
  is_postpos = 1; *pocc_type -= DOgOccMore;
  }

switch(*pocc_type) {
  case DOgOccPos: 
    IFE(occ->position=OggNin4(&p));
    break;
  case DOgOccPar:
    IFE(occ->parag_nb=OggNin4(&p));
    break;
  case DOgOccParPos:
    IFE(occ->parag_nb=OggNin4(&p));
    IFE(occ->position=OggNin4(&p));
    break;
  case DOgOccParWorPos:
    IFE(occ->parag_nb=OggNin4(&p));
    IFE(occ->word_nb_in_parag=OggNin4(&p));
    IFE(occ->position=OggNin4(&p));
    break;
  case DOgOccParSenPos:
    IFE(occ->parag_nb=OggNin4(&p));
    IFE(occ->sentence_nb=OggNin4(&p));
    IFE(occ->position=OggNin4(&p));
    break;
  case DOgOccParSenWorPos:
    IFE(occ->parag_nb=OggNin4(&p));
    IFE(occ->sentence_nb=OggNin4(&p));
    IFE(occ->word_nb_in_sentence=OggNin4(&p));
    IFE(occ->position=OggNin4(&p));
    break;
  }

if (is_postpos) {
  IFE(occ->ipostpos=OggNin4(&p));
  if (occ->ipostpos >= DOgMaxPostpos) {
    DPcSprintf(erreur,"OgStrToOcc: occ->ipostpos (%d) >= %d", occ->ipostpos, DOgMaxPostpos);
    PcErr(-1,erreur); DPcErr;
    }  
  for (i=0; i<occ->ipostpos; i++) {
    IFE(occ->postpos[i]=OggNin4(&p));
    }
  }

return(p-s);
}





PUBLIC(int) OgSprintOcc(int occ_type, struct og_occ *occ, int sB, unsigned char *B)
{
int i,iB;
sprintf(B,"occ = "); iB = strlen(B);
switch(occ_type) {
  case DOgOccPos:
    if (iB + 20 > sB) DONE;
    sprintf(B+iB, "Pos:%x", occ->position);
    break;
  case DOgOccPar: 
    if (iB + 20 > sB) DONE;
    sprintf(B+iB, "Par:%x", occ->parag_nb);
    break;
    case DOgOccParPos: 
    if (iB + 40 > sB) DONE;
    sprintf(B+iB, "Par:%x Pos:%x", occ->parag_nb, occ->position);
    break;
  case DOgOccParWorPos: 
    if (iB + 60 > sB) DONE;
    sprintf(B+iB, "Par:%x Wor:%x Pos:%x"
      , occ->parag_nb, occ->word_nb_in_parag, occ->position);
    break;
  case DOgOccParSenPos: 
    if (iB + 60 > sB) DONE;
    sprintf(B+iB, "Par:%x Sen:%x Pos:%x"
      , occ->parag_nb, occ->sentence_nb, occ->position);
    break;
  case DOgOccParSenWorPos: 
    if (iB + 60 > sB) DONE;
    sprintf(B+iB, "Par:%x Sen:%x Wor:%x Pos:%x"
      , occ->parag_nb, occ->sentence_nb, occ->word_nb_in_parag, occ->position);
    break;
  default:
    if (iB + 20 > sB) DONE;
    sprintf(B+iB, "unknow %d", occ->position);
  }

if (occ->ipostpos) {
  iB = strlen(B);
  sprintf(B+iB, " postpos (%d):", occ->ipostpos);
  iB = strlen(B);
  for (i=0; i<occ->ipostpos; i++) {
    sprintf(B+iB, " %d", occ->postpos[i]);
    }
  }

DONE;
}




