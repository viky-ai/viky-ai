/*
 *  Reading a date in string format
 *  Copyright (c) 2001-2005 Pertimm by Patrick Constant
 *  Dev : July 2001, February 2004, April 2005
 *  Version 1.2
*/
#include <loggen.h>



static int GetMonthNumber(int, char *);
static int GetZoneDifferential(int, char *, int *);



/* 
 * RFC 2616:
 * HTTP applications have historically allowed three different formats
 * for the representation of date/time stamps:
 *
 *    Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
 *    Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
 *    Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
 *
 * Pertimm: other dates can be possible:
 *    Wdy, DD-Mon-YYYY HH:MM:SS GMT  ; Cookies date
 *
 * Pertimm: RFC 822 specifies that zone is not always GMT
 *   zone        =  "UT"  / "GMT"                ; Universal Time
 *                                               ; North American : UT
 *               /  "EST" / "EDT"                ;  Eastern:  - 5/ - 4
 *               /  "CST" / "CDT"                ;  Central:  - 6/ - 5
 *               /  "MST" / "MDT"                ;  Mountain: - 7/ - 6
 *               /  "PST" / "PDT"                ;  Pacific:  - 8/ - 7
 *               /  1ALPHA                       ; Military: Z = UT;
 *                                               ;  A:-1; (J not used)
 *                                               ;  M:-12; N:+1; Y:+12
 *               / ( ("+" / "-") 4DIGIT )        ; Local differential
 *                                               ;  hours+min. (HHMM)
 *
 * The first format is preferred as an Internet standard and represents
 * a fixed-length subset of that defined by RFC 1123 [8] (an update to
 * RFC 822 [9]). The second format is in common use, but is based on the
 * obsolete RFC 850 [12] date format and lacks a four-digit year.
 * HTTP/1.1 clients and servers that parse the date value MUST accept
 * all three formats (for compatibility with HTTP/1.0), though they MUST
 * only generate the RFC 1123 format for representing HTTP-date values
 * in header fields. See section 19.3 for further information.
 *
 * This function returns the date in GMT, it is then easy to change
 * the date in localtime for example. Returns 1 if the date is found
 * and 0 otherwise.
*/

PUBLIC(int) OgGetStringDate(char *v, time_t *last_modified)
{
int ibuf=0; char buf[2048]; 
int i,c,state=1,end=0,found=0; 
int zone_differential=0; /** zero means GMT **/
struct tm ctm;

*last_modified=0;
memset(&ctm,0,sizeof(struct tm));

for (i=0; !end; i++) {
  if (v[i]==0) { c=' '; end=1; }
  else c=v[i];
  switch (state) {
    /** Weekday, **/
    case 1:
      if (isspace(c)) state=2;
      break;
    /** start of DD(1,2) or Mon(3) **/
    case 2:
      if (isspace(c)) state=2;
      else { ibuf=0; buf[ibuf++]=c; state=3; }
      break;
    /** DD(1,2) or Mon(3) **/
    case 3:
      if (isspace(c) || ispunct(c)) { 
        buf[ibuf]=0; ctm.tm_mday=atoi(buf);
        if (ctm.tm_mday==0) {
          ctm.tm_mon=GetMonthNumber(ibuf,buf);
          state=20;
          }
        else state=4; 
        }
      else buf[ibuf++]=c;
      break;
    /** start of Mon **/
    case 4:
      if (isspace(c) || ispunct(c)) state=4;
      else { ibuf=0; buf[ibuf++]=c; state=5; }
      break;
    /** Mon **/
    case 5:
      if (isspace(c) || ispunct(c)) { 
        buf[ibuf]=0; ctm.tm_mon=GetMonthNumber(ibuf,buf);
        state=6; 
        }
      else buf[ibuf++]=c;
      break;
    /** start of YY **/
    case 6:
      if (isspace(c)) state=6;
      else { ibuf=0; buf[ibuf++]=c; state=7; }
      break;
    /** YY **/
    case 7:
      if (isspace(c) || ispunct(c)) { 
        buf[ibuf]=0;
        if (ibuf==2) ctm.tm_year=atoi(buf); 
        else ctm.tm_year=atoi(buf)-1900; 
        state=8; 
        }
      else buf[ibuf++]=c;
      break;
    /** start of HH **/
    case 8:
      if (isspace(c)) state=8;
      else { ibuf=0; buf[ibuf++]=c; state=9; }
      break;
    /** HH **/
    case 9:
      if (isspace(c) || ispunct(c)) { 
        buf[ibuf]=0; ctm.tm_hour=atoi(buf); 
        state=10; 
        }
      else buf[ibuf++]=c;
      break;
    /** start of MM **/
    case 10:
      if (isspace(c)) state=10;
      else { ibuf=0; buf[ibuf++]=c; state=11; }
      break;
    /** MM **/
    case 11:
      if (isspace(c) || ispunct(c)) { 
        buf[ibuf]=0; ctm.tm_min=atoi(buf); 
        state=12; 
        }
      else buf[ibuf++]=c;
      break;
    /** start of SS **/
    case 12:
      if (isspace(c)) state=12;
      else { ibuf=0; buf[ibuf++]=c; state=13; }
      break;
    /** SS **/
    case 13:
      if (isspace(c) || ispunct(c) || c==0) { 
        buf[ibuf]=0; ctm.tm_sec=atoi(buf); 
        state=14; 
        }
      else buf[ibuf++]=c;
      break;
    /** start of zone (GMT, UT, EST, etc.) **/
    case 14:
      if (isspace(c)) state=14;
      else { ibuf=0; buf[ibuf++]=c; state=15; }
      break;
    case 15:
      if (isspace(c) || c==0) {
        buf[ibuf]=0;
        /** zone (GMT, UT, EST, etc.) **/
        IFE(GetZoneDifferential(ibuf,buf,&zone_differential));
        tzset(); /** Instanciate 'timezone' **/
        ctm.tm_isdst = 0;
        if (ctm.tm_year >= 135) *last_modified=0x7fffffff;
        else {
          *last_modified = mktime(&ctm); 
          /** Removing timezone to stay GMT **/
          *last_modified -= timezone;
          /** Removing the zone differential **/
          *last_modified -= zone_differential;
          found=1;
          }
        goto endOgGetStringDate;
        }
      else buf[ibuf++]=c;
      break;

    /** Sun Nov  6 08:49:37 1994  **/
    /** start of DD=6 **/
    case 20:
      if (isspace(c)) state=20;
      else { ibuf=0; buf[ibuf++]=c; state=21; }
      break;
    /** DD=6 **/
    case 21:
      if (isspace(c) || ispunct(c)) { 
        buf[ibuf]=0; ctm.tm_mday=atoi(buf);
        state=22; 
        }
      else buf[ibuf++]=c;
      break;
    /** start of HH **/
    case 22:
      if (isspace(c)) state=22;
      else { ibuf=0; buf[ibuf++]=c; state=23; }
      break;
    /** HH **/
    case 23:
      if (isspace(c) || ispunct(c)) { 
        buf[ibuf]=0; ctm.tm_hour=atoi(buf); 
        state=24; 
        }
      else buf[ibuf++]=c;
      break;
    /** start of MM **/
    case 24:
      if (isspace(c)) state=24;
      else { ibuf=0; buf[ibuf++]=c; state=25; }
      break;
    /** MM **/
    case 25:
      if (isspace(c) || ispunct(c)) { 
        buf[ibuf]=0; ctm.tm_min=atoi(buf); 
        state=26; 
        }
      else buf[ibuf++]=c;
      break;
    /** start of SS **/
    case 26:
      if (isspace(c)) state=26;
      else { ibuf=0; buf[ibuf++]=c; state=27; }
      break;
    /** SS **/
    case 27:
      if (isspace(c) || ispunct(c) || c==0) { 
        buf[ibuf]=0; ctm.tm_sec=atoi(buf); 
        state=28; 
        }
      else buf[ibuf++]=c;
      break;
    /** start of YY **/
    case 28:
      if (isspace(c)) state=28;
      else { ibuf=0; buf[ibuf++]=c; state=29; }
      break;
    /** YY **/
    case 29:
      if (isspace(c) || ispunct(c)) { 
        buf[ibuf]=0;
        if (ibuf==2) ctm.tm_year=atoi(buf); 
        else ctm.tm_year=atoi(buf)-1900; 
        state=14; 
        }
      else buf[ibuf++]=c;
      break;

    }
  }

endOgGetStringDate:

return(found);
}



static char *og_month[12] = { 
  "jan","feb","mar","apr","may","jun",
  "jul","aug","sep","oct","nov","dec"
  };

static int GetMonthNumber(int imonth, char *month)
{
int i; char m[10];
/** returns January by default **/
if (imonth!=3) return(1);
strcpy(m,month); PcStrlwr(m);
for (i=0; i<12; i++) {
  if (!strcmp(m,og_month[i])) return(i);
  }
return(1);
}

 

/*
 * Taken from http://wwp.greenwichmeantime.com/info/timezone.htm
*/

static int GetZoneDifferential(int izone, char *zone, int *pzone_differential)
{
int zd=0;
*pzone_differential=0;

if (!Ogstricmp(zone,"UT"))       zd=0;              /** Universal **/
else if (!Ogstricmp(zone,"GMT")) zd=0;              /** Greenwich Mean **/
else if (!Ogstricmp(zone,"UTC")) zd=0;              /** Greenwich Mean **/
else if (!Ogstricmp(zone,"WET")) zd=0;              /** Western European **/
else if (!Ogstricmp(zone,"Z"))   zd=0;              /** Military Zulu **/

else if (!Ogstricmp(zone,"CET")) zd=1*3600;         /** Central European **/
else if (!Ogstricmp(zone,"A"))   zd=1*3600;         /** Military Alpha **/

else if (!Ogstricmp(zone,"EET")) zd=2*3600;         /** Eastern European **/
else if (!Ogstricmp(zone,"B"))   zd=2*3600;         /** Military Bravo **/

else if (!Ogstricmp(zone,"BT"))  zd=3*3600;         /** Badgad **/
else if (!Ogstricmp(zone,"C"))   zd=3*3600;         /** Military Charlie **/
else if (!Ogstricmp(zone,"C*"))  zd=3*3600+1800;    /** Military Charlie **/

else if (!Ogstricmp(zone,"D"))   zd=4*3600;         /** Military Delta **/
else if (!Ogstricmp(zone,"D*"))  zd=4*3600+1800;    /** Military Delta **/

else if (!Ogstricmp(zone,"E"))   zd=5*3600;         /** Military Echo **/
else if (!Ogstricmp(zone,"E*"))  zd=5*3600+1800;    /** Military Echo **/

else if (!Ogstricmp(zone,"F"))   zd=6*3600;         /** Military Foxtrot **/
else if (!Ogstricmp(zone,"F*"))  zd=6*3600+1800;    /** Military Foxtrot **/

else if (!Ogstricmp(zone,"G"))   zd=7*3600;         /** Military Golf **/

else if (!Ogstricmp(zone,"CCT")) zd=8*3600;         /** China Coast **/
else if (!Ogstricmp(zone,"H"))   zd=8*3600;         /** Military Hotel **/

else if (!Ogstricmp(zone,"JST")) zd=9*3600;         /** Japan Standard **/
else if (!Ogstricmp(zone,"I"))   zd=9*3600;         /** Military India **/
else if (!Ogstricmp(zone,"I*"))  zd=9*3600+1800;    /** Military India **/

else if (!Ogstricmp(zone,"GST")) zd=10*3600;        /** Guam Standard **/
else if (!Ogstricmp(zone,"K"))   zd=10*3600;        /** Military Kilo **/
else if (!Ogstricmp(zone,"K*"))  zd=10*3600+1800;   /** Military Kilo **/

else if (!Ogstricmp(zone,"L"))   zd=11*3600;        /** Military Lima **/
else if (!Ogstricmp(zone,"L*"))  zd=11*3600+1800;   /** Military Lima **/

else if (!Ogstricmp(zone,"IDLE")) zd=12*3600;       /** International Date Line East **/
else if (!Ogstricmp(zone,"NZST")) zd=12*3600;       /** New Zealand Standard **/
else if (!Ogstricmp(zone,"M"))    zd=12*3600;       /** Military Mike **/
else if (!Ogstricmp(zone,"M*"))   zd=12*3600+1800;  /** Military Mike **/

else if (!Ogstricmp(zone,"WAT")) zd=-1*3600;        /** West Africa **/
else if (!Ogstricmp(zone,"N"))   zd=-1*3600;        /** Military November **/

else if (!Ogstricmp(zone,"AT"))  zd=-2*3600;        /** Azores **/
else if (!Ogstricmp(zone,"O"))   zd=-2*3600;        /** Military Oscar **/

else if (!Ogstricmp(zone,"P"))   zd=-3*3600;        /** Military Papa **/
else if (!Ogstricmp(zone,"P*"))  zd=-3*3600-1800;   /** Military Papa **/

else if (!Ogstricmp(zone,"AST")) zd=-4*3600;        /** Atlantic Standard **/
else if (!Ogstricmp(zone,"Q"))   zd=-4*3600;        /** Military Quebec **/

else if (!Ogstricmp(zone,"EST")) zd=-5*3600;        /** Eastern Standard **/
else if (!Ogstricmp(zone,"R"))   zd=-5*3600;        /** Military Romeo **/

else if (!Ogstricmp(zone,"CST")) zd=-6*3600;        /** Central Standard **/
else if (!Ogstricmp(zone,"S"))   zd=-6*3600;        /** Military Sierra **/

else if (!Ogstricmp(zone,"MST")) zd=-7*3600;        /** Mountain Standard **/
else if (!Ogstricmp(zone,"T"))   zd=-7*3600;        /** Military Tango **/

else if (!Ogstricmp(zone,"PST")) zd=-8*3600;        /** Pacific Standard **/
else if (!Ogstricmp(zone,"U"))   zd=-8*3600;        /** Military Uniform **/
else if (!Ogstricmp(zone,"U*"))  zd=-8*3600-1800;   /** Military Uniform **/

else if (!Ogstricmp(zone,"V"))   zd=-9*3600;        /** Military Victor **/
else if (!Ogstricmp(zone,"V*"))  zd=-9*3600-1800;   /** Military Victor **/

else if (!Ogstricmp(zone,"AHST")) zd=-10*3600;      /** Alaska-Hawaii Standard **/
else if (!Ogstricmp(zone,"CAT"))  zd=-10*3600;      /** Central Alaska **/
else if (!Ogstricmp(zone,"HST"))  zd=-10*3600;      /** Hawaii Standard **/
else if (!Ogstricmp(zone,"W"))    zd=-10*3600;      /** Military Whiskey **/

else if (!Ogstricmp(zone,"NT"))  zd=-11*3600;       /** Nome **/
else if (!Ogstricmp(zone,"X"))   zd=-11*3600;       /** Military X-ray **/

else if (!Ogstricmp(zone,"IDLW")) zd=-12*3600;      /** International Date Line West **/
else if (!Ogstricmp(zone,"Y"))    zd=-12*3600;      /** Military Yankee **/

else if (izone==5) {
  int sign=1,hour,minute; 
  char shour[9], sminute[9];
  
  if (zone[0]=='+') sign=1;
  else if (zone[0]=='-') sign=-1;
  shour[0]=zone[1]; shour[1]=zone[2]; shour[2]=0;
  sminute[0]=zone[3]; sminute[1]=zone[4]; sminute[2]=0;
  
  hour=atoi(shour); minute=atoi(sminute);
  zd = sign * (hour*3600 + minute*60);
  }
*pzone_differential=zd;

DONE;
}








