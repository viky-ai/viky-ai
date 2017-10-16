/*
 *	This function tries to identify a tag
 *	Copyright (c) 1998-2000	Ogmios by Patrick Constant
 *	Dev : October 1998, April 2000, August 2008
 *	Version 1.2
*/
#include <loggen.h>




/*
 *  Identification of a tag: start with '<' and end with '>'
 *  <!-- contains anything but finishes by a --> (returns 2)
 *  returns 1 if it is a tag and 0 if not.
 *  grammar for a tag is quite simple:
 *  name name1 = " anything1" name2 = " anything2" ... 
 *  returns 3 if doesn't know.
*/

PUBLIC(int) OgIsTag(int unicode, int ibuffer, char *buffer, int *length)
{
int idoctype,i=0,c,state,quote;
state=1; *length=0;

if (unicode) idoctype=16;
else idoctype=8;

for (i=0; i<ibuffer; i++) {
  if (unicode) { c = buffer[i+1]; i++; }
  else c = buffer[i];
  switch(state) {
    
    case 1: if (c=='<') state=2; else return(0); 
      break;
    
    /** no space just after '<', XML accepts "_:-." **/
    case 2:
      if (PcIsspace(c)) return(0);
      else if (PcIsalnum(c) || c=='_' || c==':'|| c=='-'|| c=='.') state=3;
      else if (c=='/') state=3;
      else if (c=='!') state=20;
      else if (c=='?') state=25;
      else return(0); 
      break; 

    /** in tag name **/
    case 3:
      if (c=='>') { *length=i+1; return(1); }
      else if (PcIsspace(c)) state=4;
      else if (PcIsalnum(c) || c=='_' || c==':'|| c=='-'|| c=='.') state=3;
      else if (c=='/') state=30;
      else return(0);
      break;

    /** space after tag name **/
    case 4:
      if (c=='>') { *length=i+1; return(1); }
      else if (PcIsspace(c)) state=4;
      else if (PcIsalnum(c) || c=='_' || c==':'|| c=='-'|| c=='.') state=5;
      else if (c=='/') state=30;
      else return(0);
      break;
    
    /** in variable name, some have ':' in them **/
    case 5:
      if (c=='>') { *length=i+1; return(1); }
      else if (c=='=') state=7;
      else if (PcIsspace(c)) state=6;
      else if (PcIsalnum(c) || c=='_'  || c==':'|| c=='-'|| c=='.') state=5;
      else if (c=='-') state=5;
      else if (c=='/') state=30;
      else return(0);
      break;

    /** in space after variable name **/
    /** nowrap has no attributes **/
    case 6:
      if (c=='>') { *length=i+1; return(1); }
      else if (c=='=') state=7;
      else if (PcIsspace(c)) state=6;
      else if (PcIsalnum(c) || c=='_'  || c==':'|| c=='-'|| c=='.') state=4;
      else if (c=='/') state=30;
      else return(0);
      break;
    
    /** after '=' **/
    case 7:
      if (c=='>') return(0);
      else if (c=='"') { quote=2; state=10; }
      else if (c=='\'') { quote=1; state=10; }
      else if (PcIsspace(c)) state=8;
      //else if (PcIsalnum(c) || c=='_' || c=='-' || c=='+' || c=='#' || c=='.' || c=='/') state=9;
      //else return(0);
      /** All non space chars are accepted as values **/
      else state=9;
      break;
    
    /** in space after '=' **/
    case 8:
      if (c=='>') return(0);
      else if (c=='"') { quote=2; state=10; }
      else if (c=='\'') { quote=1; state=10; }
      else if (PcIsspace(c)) state=8;
      //else if (PcIsalnum(c) || c=='_' || c=='-' || c=='+' || c=='#' || c=='.' || c=='/') state=9;
      //else return(0);
      /** All non space chars are accepted as values **/
      else state=9;
      break;
    
    /* In variable value without '"' or '\''. The followings char have been seen ":,", 
     * so we decide to simply say it stops with a space char.
    */
    case 9:
      if (c=='>') { *length=i+1; return(1); }
      else if (PcIsspace(c)) state=4;
      else if (c=='/') state=29;
      else state=9;
      break;
    
    /** in variable with '"' or '\'' **/
    case 10:
      //if (c=='>') return(0);
      if (c=='"') { if (quote==2) state=4; else state=10; }
      else if (c=='\'') { if (quote==1) state=4; else state=10; }
      else state=10;
      break;

    /** 20 to 21 handles <!-- or <!doctype **/
    case 20:
      if (c=='-') state=21;
      else if (i+idoctype < ibuffer) {
        int j; char doctype[128];
        if (unicode) { OgUtoA(buffer+i-1,doctype,idoctype); }
        else { memcpy(doctype,buffer+i,8); doctype[8] = 0; }
        PcStrlwr(doctype);
        if (!memcmp(doctype,"doctype ",8)) {
          for (j=i; j<ibuffer; j++) if (buffer[j]=='>') { *length=j+1; return(1); }
          *length=ibuffer; return(3);
          }
        else return(0);
        }
      else return(0); 
      break; 

    case 21:
      if (c=='-') state=22;
      else return(0); 
      break; 

    /** in the comment **/
    case 22:
      if (c=='-') state=23;
      else state=22; 
      break; 

    /** 23 to 24 handles --> **/
    case 23:
      if (c=='-') state=24;
      else state=22; 
      break; 

    case 24:
      if (c=='>') { *length=i+1; return(2); }
      else if (c=='-') state=24;
      else state=22; 
      break; 

    /** 25 to 28 handles <?xml **/
    case 25:
      if (c=='x') state=26;
      else return(0); 
      break; 

    case 26:
      if (c=='m') state=27;
      else return(0); 
      break; 

    case 27:
      if (c=='l') state=28;
      else return(0); 
      break; 

    /** in the ?xml tag **/
    case 28:
      if (c=='>') { *length=i+1; return(1); }
      else state=28; 
      break; 

    /** Coming from state 9, not sure we are at end of XML tag **/
    case 29:
      if (c=='>') { *length=i+1; return(1); }
      else if (PcIsspace(c)) state=4;
      else if (c=='/') state=29;
      else state=9; 
      break; 

    /** In XML, there is a '/' at the end of some tags **/
    case 30:
      if (c=='>') { *length=i+1; return(1); }
      else if (PcIsspace(c)) state=30;
      else return(0); 
      break; 
    }
  }
*length=i;
return(3);
}



