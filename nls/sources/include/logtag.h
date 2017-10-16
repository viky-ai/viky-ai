/*
 *	Simply for reading tagctrl.dat and getting the corresponding structure
 *	Copyright (c) 1999-2000	Ogmios by Patrick Constant
 *	Dev: August 1999, May 2000, April 2001, December 2003
 *  Dev: May,October 2004, May 2005
 *	Version 1.6
*/
#ifndef _LOGTAGALIVE_
#include <lpcosys.h>
#include <lpcmcr.h>


#define DOgNbField   9
#define DOgDefaultField   "<default>"

#define DOgMaxCtrlTag       256
#define DOgMaxCtrlTagLength 100

#define DOgTagContentTypeNil        0x0
#define DOgTagContentTypeEntity     0x1
#define DOgTagContentTypeFileEntity 0x2
#define DOgTagContentTypeNumber     0x4
#define DOgTagContentTypeDate       0x8
#define DOgTagContentTypeXref       0x10
#define DOgTagContentTypeTitle      0x20
#define DOgTagContentTypeSingle     0x40

#define DOgTitlePriority            1000    /* value for tag title within a file */
#define DOgDefaultTitlePriority     2000    /* default value for tag declared in tag_ctrl.dat as title within a file */
#define DOgXmlTitlePriority         3000    /* value for tag title in an XML declaration file */
#define DOgDefaultXmlTitlePriority  4000    /* default value for tag declared as title in a control XML file */


struct tag_ctrlt {
  int iname; unsigned char name[DOgMaxCtrlTagLength];
  unsigned char string[DOgMaxCtrlTagLength];
  unsigned length, value, weight, content_type;
  unsigned title_priority;
  unsigned keep_tag:        1;
  unsigned keep_text:       1;
  unsigned cut_word:        1;
  unsigned cut_expr:        1;
  unsigned libre:          12;
  };

struct tag_ctrl {
  int nb_tags;
  struct tag_ctrlt t[DOgMaxCtrlTag];
  };


DEFPUBLIC(int) ReadTagCtrl(pr_(char *) pr_(struct tag_ctrl *) pr(char *));
DEFPUBLIC(int) PrintTagCtrl(pr_(struct tag_ctrl *) pr(char *));
DEFPUBLIC(char *) OgGetTagContentTypeString(pr_(int) pr(char *));


#define _LOGTAGALIVE_
#endif
