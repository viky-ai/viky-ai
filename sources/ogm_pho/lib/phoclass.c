/*
 *	Rules classes
 *  Copyright (c) 2008 Pertimm by G.Logerot
 *  Dev : May 2008
 *  Version 1.0
*/
#include "ogm_pho.h"

STATICF(int) AllocChar_class(pr_(struct og_ctrl_pho *) pr(struct char_class **));


int ClassLog(ctrl_pho,filename)
struct og_ctrl_pho *ctrl_pho;
char *filename;
{
struct char_class *char_class;
char B1[DPcPathSize],B2[DPcPathSize];
int i,k,iB1,iB2;
FILE *fd;


IFn(fd=fopen(filename,"wb")) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"ClassLog: impossible to fopen '%s'",filename);
  DONE;
  }

/* code uni (2)+ "Number::[[character]]::characters_list\n" (39) */
fwrite("\xFE\xFF\0N\0u\0m\0b\0e\0r\0:\0:\0[\0[\0c\0h\0a\0r\0a\0c\0t\0e\0r\0]\0]\0:\0:\0c\0h\0a\0r\0a\0c\0t\0e\0r\0s\0_\0l\0i\0s\0t\0\n"
      ,sizeof(unsigned char),80,fd);

for(i=0;i<ctrl_pho->Char_classUsed;i++) {
  char_class = ctrl_pho->Char_class + i;
  for(k=0;k<char_class->number;k++) {
    iB1=sprintf(B1,"%d::[[",i);
    IFE(OgCpToUni(iB1,B1,DPcPathSize,&iB2,B2,DOgCodePageANSI,0,0));
    fwrite(B2,sizeof(unsigned char),iB2,fd);
    fwrite(char_class->character,sizeof(unsigned char),2,fd);
    fwrite("\0]\0]\0:\0:",sizeof(unsigned char),8,fd);
    fwrite(ctrl_pho->BaClass+char_class->start+k*2,sizeof(unsigned char),2,fd);
    fwrite("\0\n",sizeof(unsigned char),2,fd);
    }
  }

fclose(fd);

DONE;
}





int ClassCreate(ctrl_pho,b)
struct og_ctrl_pho *ctrl_pho;
unsigned char *b;
{
struct char_class *char_class;
int Ichar_class;

IFE(Ichar_class=AllocChar_class(ctrl_pho,&char_class));
memcpy(char_class->character,b,2);
char_class->start=ctrl_pho->BaClassUsed;

return(Ichar_class);
}




int ClassAddC(ctrl_pho,Ichar_class,b)
struct og_ctrl_pho *ctrl_pho;
int Ichar_class;
unsigned char *b;
{
struct char_class *char_class=ctrl_pho->Char_class+Ichar_class;
IFE(PhoAppendBaClass(ctrl_pho,2,b));
char_class->number++;
DONE;
}






STATICF(int) AllocChar_class(ctrl_pho,pchar_class)
struct og_ctrl_pho *ctrl_pho;
struct char_class **pchar_class;
{
char erreur[DOgErrorSize];
struct char_class *char_class = 0;
int i=ctrl_pho->Char_classNumber;

beginAllocChar_class:

if (ctrl_pho->Char_classUsed < ctrl_pho->Char_classNumber) {
  i = ctrl_pho->Char_classUsed++; 
  }

if (i == ctrl_pho->Char_classNumber) {
  unsigned a, b; struct char_class *og_l;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory) {
    OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"AllocChar_class: max Char_class number (%d) reached", ctrl_pho->Char_classNumber);
    }
  a = ctrl_pho->Char_classNumber; b = a + (a>>2) + 1;
  IFn(og_l=(struct char_class *)malloc(b*sizeof(struct char_class))) {
    sprintf(erreur,"AllocChar_class: malloc error on Char_class");
    OgErr(ctrl_pho->herr,erreur); DPcErr;
    }

  memcpy( og_l, ctrl_pho->Char_class, a*sizeof(struct char_class));
  DPcFree(ctrl_pho->Char_class); ctrl_pho->Char_class = og_l;
  ctrl_pho->Char_classNumber = b;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory) {
    OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"AllocChar_class: new Char_class number is %d\n", ctrl_pho->Char_classNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocChar_class: Char_classNumber reached (%d)",ctrl_pho->Char_classNumber);
  OgErr(ctrl_pho->herr,erreur); DPcErr;
#endif

  goto beginAllocChar_class;
  }

char_class = ctrl_pho->Char_class + i;
memset(char_class,0,sizeof(struct char_class));

if (pchar_class) *pchar_class = char_class;
return(i);
}