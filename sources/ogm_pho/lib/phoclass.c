/*
 *  Rules classes
 *  Copyright (c) 2008 Pertimm by G.Logerot
 *  Dev : May 2008
 *  Version 1.0
*/
#include "ogm_pho.h"

static int AllocChar_class(struct lang_context *lang_context, struct char_class **pchar_class);


og_status ClassLog(struct lang_context *lang_context, char *filename)
{
  struct og_ctrl_pho *ctrl_pho = lang_context->ctrl_pho;

  FILE *fd = fopen(filename,"wb");

  IFn(fd)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "ClassLog: impossible to fopen '%s'", filename);
    DONE;
  }

  /* code uni (2)+ "Number::[[character]]::characters_list\n" (39) */
  fwrite(
      "\xFE\xFF\0N\0u\0m\0b\0e\0r\0:\0:\0[\0[\0c\0h\0a\0r\0a\0c\0t\0e\0r\0]\0]\0:\0:\0c\0h\0a\0r\0a\0c\0t\0e\0r\0s\0_\0l\0i\0s\0t\0\n",
      sizeof(unsigned char), 80, fd);

  for (int i = 0; i < lang_context->Char_classUsed; i++)
  {
    struct char_class *char_class = lang_context->Char_class + i;
    for (int k = 0; k < char_class->number; k++)
    {
      char B1[DPcPathSize];
      int iB1 = sprintf(B1, "%d::[[", i);
      char B2[DPcPathSize];
      int iB2 = 0;
      IFE(OgCpToUni(iB1,B1,DPcPathSize,&iB2,B2,DOgCodePageANSI,0,0));
      fwrite(B2, sizeof(unsigned char), iB2, fd);
      fwrite(char_class->character, sizeof(unsigned char), 2, fd);
      fwrite("\0]\0]\0:\0:", sizeof(unsigned char), 8, fd);
      fwrite(lang_context->BaClass + char_class->start + k * 2, sizeof(unsigned char), 2, fd);
      fwrite("\0\n", sizeof(unsigned char), 2, fd);
    }
  }

  fclose(fd);

  DONE;
}


int ClassCreate(struct lang_context *lang_context, unsigned char *b)
{
  struct char_class *char_class = NULL;
  int Ichar_class = AllocChar_class(lang_context, &char_class);
  IFE(Ichar_class);
  memcpy(char_class->character, b, 2);
  char_class->start = lang_context->BaClassUsed;

  return (Ichar_class);
}


og_status ClassAddC(struct lang_context *lang_context, int Ichar_class, unsigned char *b)
{
  struct char_class *char_class = lang_context->Char_class + Ichar_class;
  IFE(PhoAppendBaClass(lang_context, 2, b));
  char_class->number++;
  DONE;
}


static int AllocChar_class(struct lang_context *lang_context, struct char_class **pchar_class)
{
  struct og_ctrl_pho *ctrl_pho = lang_context->ctrl_pho;

  struct char_class *char_class = 0;
  int i=lang_context->Char_classNumber;

  beginAllocChar_class:

  if (lang_context->Char_classUsed < lang_context->Char_classNumber)
  {
    i = lang_context->Char_classUsed++;
  }

  if (i == lang_context->Char_classNumber)
  {
    unsigned a, b; struct char_class *og_l;

    if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory)
    {
      OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"AllocChar_class: max Char_class number (%d) reached", lang_context->Char_classNumber);
    }
    a = lang_context->Char_classNumber; b = a + (a>>2) + 1;
    IFn(og_l=(struct char_class *)malloc(b*sizeof(struct char_class)))
    {
      char erreur[DOgErrorSize];
      sprintf(erreur,"AllocChar_class: malloc error on Char_class");
      OgErr(ctrl_pho->herr,erreur); DPcErr;
    }

    memcpy( og_l, lang_context->Char_class, a*sizeof(struct char_class));
    DPcFree(lang_context->Char_class); lang_context->Char_class = og_l;
    lang_context->Char_classNumber = b;

    if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory)
    {
      OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"AllocChar_class: new Char_class number is %d\n", lang_context->Char_classNumber);
    }

    #ifdef DOgNoMainBufferReallocation
        sprintf(erreur,"AllocChar_class: Char_classNumber reached (%d)",ctrl_pho->Char_classNumber);
        OgErr(ctrl_pho->herr,erreur); DPcErr;
    #endif

    goto beginAllocChar_class;
  }

  char_class = lang_context->Char_class + i;
  memset(char_class,0,sizeof(struct char_class));

  if (pchar_class) *pchar_class = char_class;
  return(i);
}
