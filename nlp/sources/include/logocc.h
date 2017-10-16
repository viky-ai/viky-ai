/*
 *	This is include for occurrences types and values
 *	Copyright (c) 1999-2001	Pertimm, Inc.
 *	Dev : November 1999, September 2001
 *	Version 1.1
 *
*/

#ifndef _LOGOCCALIVE_
#include <lpcosys.h>

#define DOgOccPos           1
#define DOgOccPar           2
#define DOgOccParPos        3
#define DOgOccParWorPos     4
#define DOgOccParSenPos     5
#define DOgOccParSenWorPos  6

/* The bit that indicates that the occurrence is encoded
 * with more information after the usual encoding. This added 
 * information starts with a number that gives the number
 * of numbers that are encoded afterwards. This number is
 * 'ipostpos' and the number can be found in postpos.
 * So the type of the occ is a number that should not use this bit.
 * for example 0 to 0x3f and then 0x80 to 0xbf, etc...
 * For the moment we only have 6 types out of which 99 % is type 3.
*/
#define DOgOccMore          0x40

/** Max number of post-positions for PDF-like extracted format **/
#define DOgMaxPostpos   5

struct og_occ {
  unsigned char *word;
  int word_length;
  int no_occ;
  int position;
  int parag_nb;
  int sentence_nb;
  int word_nb_in_parag;
  int word_nb_in_sentence;
  int ipostpos, postpos[DOgMaxPostpos];
  };


DEFPUBLIC(int) OgOccToStr(pr_(int) pr_(struct og_occ *) 
                          pr_(int) pr(unsigned char *));

DEFPUBLIC(int) OgStrToOcc(pr_(int) pr_(unsigned char *) 
                          pr_(int *) pr(struct og_occ *));

DEFPUBLIC(int) OgSprintOcc(pr_(int) pr_(struct og_occ *) 
                          pr_(int) pr(unsigned char *));

#define _LOGOCCALIVE_

#endif

