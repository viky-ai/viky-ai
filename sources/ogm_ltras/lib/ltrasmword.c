/*
 *	Handling trf structures
 *	Copyright (c) 2009-2010 Pertimm by Patrick Constant
 *	Dev: November 2009, February 2010
 *	Version 1.0
*/
#include "ogm_ltras.h"


static int LtrasAllocWord(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_word **pword);




int LtrasAddWord(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_trf *trf
  , int string_length, unsigned char *string, int frequency, int base_frequency
  , int start_position, int length_position, int language)
{
int Iword; struct og_ltra_word *word;

if (string_length > DPcPathSize / 2) string_length = DPcPathSize / 2;

IFE(Iword=LtrasAllocWord(ctrl_ltras, trfs, &word));
if (trf->start_word < 0) {
  trf->start_word=Iword;
  trf->nb_words=1;
  }
else trf->nb_words++;

word->start=trfs->BaUsed; word->length=string_length;
/** this is done because string can be from trfs->Ba **/
IFE(LtrasTrfsTestReallocBa(ctrl_ltras,trfs,string_length));
IFE(LtrasTrfsAppendBa(ctrl_ltras,trfs,string_length,string));
word->frequency=frequency;
word->base_frequency=base_frequency;
word->start_position=start_position;
word->length_position=length_position;
word->language=language;

DONE;
}




static int LtrasAllocWord(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_word **pword)
{
char erreur[DOgErrorSize];
struct og_ltra_word *word = 0;
int i=trfs->WordNumber;

beginAllocWord:

if (trfs->WordUsed < trfs->WordNumber) {
  i = trfs->WordUsed++; 
  }

if (i == trfs->WordNumber) {
  unsigned a, b; struct og_ltra_word *og_trf;

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      , "AllocWord: max Word number (%d) reached"
      , trfs->WordNumber);
    }
  a = trfs->WordNumber; b = a + (a>>2) + 1;
  IFn(og_trf=(struct og_ltra_word *)malloc(b*sizeof(struct og_ltra_word))) {
    sprintf(erreur,"AllocWord: malloc error on Word");
    OgErr(ctrl_ltras->herr,erreur); DPcErr;
    }

  memcpy( og_trf, trfs->Word, a*sizeof(struct og_ltra_word));
  DPcFree(trfs->Word); trfs->Word = og_trf;
  trfs->WordNumber = b;

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      , "AllocWord: new Word number is %d\n", trfs->WordNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocWord: WordNumber reached (%d)",trfs->WordNumber);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
#endif

  goto beginAllocWord;
  }

word = trfs->Word + i;
memset(word,0,sizeof(struct og_ltra_word));
word->start_position=(-1);

if (pword) *pword = word;
return(i);
}


