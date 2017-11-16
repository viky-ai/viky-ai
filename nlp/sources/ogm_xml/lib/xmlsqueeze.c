/*
 *	Utility functions to format XML buffers
 *	Copyright (c) 2009 Pertimm by Gabriel Ripoche
 *	Dev: Jan 2009
 *	Version 1.0
*/
#include "ogm_xml.h"


#define DOgXMLTrimNo     0
#define DOgXMLTrimYes    1
#define DOgXMLTrimMaybe  2



/*
 * Remove all extra spaces and puts entire XML buffer on one line.
 * The function can be used with the same buffer as bin and bout,
 * formatting will then happen in place. 
 */
PUBLIC(int) OgXmlSqueeze(int ibin, unsigned char *bin, int *ibout, unsigned char *bout) {
int trim, copy;
int c, i, j, k;

trim = DOgXMLTrimYes;
copy = 0;
j = 0;
k = 0;
for (i = 0; i < ibin; i++) {
  c = bin[i];
  switch (c) {
    case '>':
      trim = DOgXMLTrimYes;
      copy = 1;
      break;
    case '\n':
    case '\r':
      copy = 0;  // always remove
      break;
    case '\t':
      c = ' ';
      // do NOT break, handle like ' '
    case ' ':
      switch (trim) {
        case DOgXMLTrimYes:  // ">  <" or ">  blah"
          copy = 0;
          break;
        case DOgXMLTrimNo:  // potential case "blah  <"
          trim = DOgXMLTrimMaybe;
          k = j;
          copy = 1;
          break;
        case DOgXMLTrimMaybe:  // potential case "blah  <" (cont'd)
          copy = 1;
          break;
        }
      break;
    case '<':  // ">  <"  or "blah  <"
      if (trim == DOgXMLTrimMaybe) {  // case "blah  <"
        j = k;
        }
      trim = DOgXMLTrimNo;
      copy = 1;
      break;
    default:  // all non-XML and non-space chars
      trim = DOgXMLTrimNo;
      copy = 1;
    }
  if (copy) {
    bout[j] = c;
    j++;
    }
  }

*ibout = j;

DONE;
}


