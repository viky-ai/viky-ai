/*
 *  Handling real numbers and dates per Pertimm design
 *  Copyright (c) 2001,2004 Pertimm
 *  Dev : September 2001, July 2004
 *  Version 1.1
*/
#include <loggen.h>



static int OgAtoOgDate1(int, char *, struct og_date *);




PUBLIC(int) OgGmtToOgDate(time_t time, struct og_date *date)
{
struct tm *gmt = gmtime(&time);
if (gmt==0) { 
  char erreur[DPcSzErr];
  DPcSprintf(erreur,"OgGmtToOgDate: gmtime error on %lx",time);
  PcErr(-1,erreur); DPcErr;
  }
memset(date,0,sizeof(struct og_date));

date->iyear = 1900 + gmt->tm_year;
date->month = 1 + gmt->tm_mon;
date->mday = gmt->tm_mday;
date->hour = gmt->tm_hour;
date->minute = gmt->tm_min;
date->isecond = gmt->tm_sec;

DONE;
}





PUBLIC(int) OgLotToOgDate(time_t time, struct og_date *date)
{
struct tm *lot = localtime(&time);
if (lot==0) { 
  char erreur[DPcSzErr];
  DPcSprintf(erreur,"OgLotToOgDate: localtime error on %lx",time);
  PcErr(-1,erreur); DPcErr;
  }
memset(date,0,sizeof(struct og_date));

date->iyear = 1900 + lot->tm_year;
date->month = 1 + lot->tm_mon;
date->mday = lot->tm_mday;
date->hour = lot->tm_hour;
date->minute = lot->tm_min;
date->isecond = lot->tm_sec;

DONE;
}





PUBLIC(int) OgDateToTime_t(struct og_date *date, time_t *time)
{
struct tm cgmt;
struct tm *gmt = &cgmt;
char erreur[DPcSzErr];


if (date->is_dyear) { 
  DPcSprintf(erreur,"OgDateToTime_t: is_dyear not done yet");
  PcErr(-1,erreur); DPcErr;
  }
if (date->is_dsecond) { 
  DPcSprintf(erreur,"OgDateToTime_t: is_dsecond not done yet");
  PcErr(-1,erreur); DPcErr;
  }
memset(gmt,0,sizeof(struct tm));
gmt->tm_year = date->iyear - 1900;
gmt->tm_mon = date->month - 1;
gmt->tm_mday = date->mday;
gmt->tm_hour = date->hour;
gmt->tm_min = date->minute;
gmt->tm_sec = date->isecond;
gmt->tm_isdst = -1;

if ((*time = mktime(gmt))==(time_t)-1) {
  sprintf(erreur,"OgDateToTime_t: mktime error");
  PcErr(-1,erreur); DPcErr;
  }
DONE;
}




/*  the buffer pr (size ipr) is filled with concatenation of Pertimm numbers
 *  for year, month, yday hour min and seconds. hours and minutes are 
 *  1 shifted to avoid a zero.
*/
PUBLIC(int) OgDateToOgReal(struct og_date *date, int sor, unsigned char *or)
{
int ior,nor;
char erreur[DPcSzErr];
if (date->is_dyear) {
  IFE(ior=OgDoubleToOgReal(date->dyear,sor,or));
  }
else {
  IFE(ior=OgIntToOgReal(date->iyear,sor,or));
  }

if (sor < ior + 4 + 13) {
  DPcSprintf(erreur,"OgDateToOgReal: sor = %d < %d",sor,ior + 4 + 13);
  PcErr(-1,erreur); DPcErr;
  }

or[ior++]=0;
or[ior++]=date->month;
or[ior++]=date->mday;
or[ior++]=date->hour+1;
or[ior++]=date->minute+1;
if (date->is_dsecond) {
  IFE(nor=OgDoubleToOgReal(date->dsecond,sor-ior,or+ior));
  }
else {
  IFE(nor=OgIntToOgReal(date->isecond,sor-ior,or+ior));
  }
ior+=nor;
return(ior);
}






PUBLIC(int) OgDateLength(unsigned char *or)
{
int ior;
struct og_date date;
IFE(ior=OgRealToOgDate(or,&date));
return(ior);
}






PUBLIC(int) OgRealToOgDate(unsigned char *or, struct og_date *date)
{
int ior,lor;
memset(date,0,sizeof(struct og_date));
IFE(ior=OgRealToInt(or,&date->iyear,0)); ior++;
if (or[ior]==0) return(ior); date->month = or[ior++];
if (or[ior]==0) return(ior); date->mday = or[ior++];
if (or[ior]==0) return(ior); date->hour = or[ior++]-1;
if (or[ior]==0) return(ior); date->minute = or[ior++]-1;
IFE(lor=OgRealToInt(or+ior,&date->isecond,0));
ior += lor;
return(ior);
}




PUBLIC(int) OgRealToOgDateString(unsigned char *or, char *sdate)
{
int ior,lor;
struct og_dispatched_real dr;
struct og_date cdate,*date=&cdate;
memset(date,0,sizeof(struct og_date));

IFE(ior=OgRealToStruct(or,&dr)); ior++;
if (or[ior]) {
  date->month = or[ior++];
  if (or[ior]) {
    date->mday = or[ior++];
    if (or[ior]) {
      date->hour = or[ior++]-1;
      if (or[ior]) {
        date->minute = or[ior++]-1;
        IFE(lor=OgRealToInt(or+ior,&date->isecond,0));
        ior += lor;
        }
      }
    }
  }
if (dr.infinite > 0) {
  strcpy(sdate,"Plus Infinite");
  }
else if (dr.infinite == 0) {
  IFE(OgRealToInt(or,&date->iyear,0));
  sprintf(sdate, "%d/%d/%d-%d:%d:%d"
    , date->iyear, date->month, date->mday
    , date->hour, date->minute, date->isecond);
  }
else { /* infinite < 0 */
  strcpy(sdate,"Minus Infinite");
  }

return(ior);
}





PUBLIC(int) OgGmtToOgReal(time_t time, int sor, unsigned char *or)
{
struct og_date date;
IFE(OgGmtToOgDate(time,&date));
return(OgDateToOgReal(&date,sor,or));
}




PUBLIC(int) OgLotToOgReal(time_t time, int sor, unsigned char *or)
{
struct og_date date;
IFE(OgLotToOgDate(time,&date));
return(OgDateToOgReal(&date,sor,or));
}




PUBLIC(int) OgRealToTime_t(unsigned char *or, time_t *time)
{
int ior;
struct og_date date;
IFE(ior=OgRealToOgDate(or,&date));
OgDateToTime_t(&date,time);
return(ior);
}




PUBLIC(int) OgAtoOgDate(int is, char *s, struct og_date *date)
{
int i,inumber=0,state;
char erreur[DPcSzErr];
char number[100];
int nth = 0;

/** Default values are <year>/01/01/00/00/00 **/
memset(date,0,sizeof(struct og_date));

state = 1;
for (i=0; i<=is; i++) {
  int c = s[i];  
  switch(state) {
    case 1: 
      if (i==is || isspace(c));
      else {
        inumber=0; number[inumber++] = c;
        state = 2; 
        }
      break;
    case 2: 
      if (i==is || isspace(c)) {
        number[inumber]=0;  
        IFE(OgAtoOgDate1(nth++,number,date));
        state = 3;
        }
      else if (c == '/') {
        number[inumber]=0;  
        IFE(OgAtoOgDate1(nth++,number,date));
        state = 1;
        }
      else number[inumber++] = c;
      break;
    case 3: 
      if (i==is || isspace(c));
      else if (c == '/') state = 1;
      else {
        sprintf(erreur,"OgAtoOgDate: malformed date '%.*s'",is,s);
        PcErr(-1,erreur); DPcErr;
        }
      break;
    }
  }

DONE;
}



/*
 *  Fills-in a field of struct og_date.
*/

static int OgAtoOgDate1(int nth, char *number, struct og_date *date)
{
int n,num;
char erreur[DPcSzErr];

switch(nth) {

  case 0:
    n = atoi(number);
    date->iyear = n;
    break;

  case 1:
    n = atoi(number);
    if (1 <= n && n <= 12) num = n;
    else  {
      sprintf(erreur,"OgAtoOgDate1: bad month '%s'",number);
      PcErr(-1,erreur); DPcErr;
      }   
    date->month = num;
    break;

  case 2:
    n = atoi(number);
    if (1 <= n && n <= 31) num = n;
    else  {
      sprintf(erreur,"OgAtoOgDate1: bad day '%s'",number);
      PcErr(-1,erreur); DPcErr;
      }   
    date->mday = num;
    break;

  case 3:
    n = atoi(number);
    if (0 <= n && n <= 23) num = n;
    else  {
      sprintf(erreur,"OgAtoOgDate1: bad hour '%s'",number);
      PcErr(-1,erreur); DPcErr;
      }   
    date->hour = num;
    break;

  case 4:
    n = atoi(number);
    if (0 <= n && n <= 59) num = n;
    else  {
      sprintf(erreur,"OgAtoOgDate1: bad minute '%s'",number);
      PcErr(-1,erreur); DPcErr;
      }   
    date->minute = num;
    break;

  case 5:
    n = atoi(number);
    if (0 <= n && n <= 59) num = n;
    else  {
      sprintf(erreur,"OgAtoOgDate1: bad second '%s'",number);
      PcErr(-1,erreur); DPcErr;
      }   
    date->isecond = num;
    break;
  
  default:
    sprintf(erreur,"OgAtoOgDate1: too many numbers at '%s'",number);
    PcErr(-1,erreur); DPcErr;

  }
DONE;
}





PUBLIC(int) OgAtoOgDateReal(int is, char *s, int sor, unsigned char *or)
{
int ior; struct og_date date;
IFE(OgAtoOgDate(is,s,&date));
IFE(ior=OgDateToOgReal(&date,sor,or));
return(ior);
}




