/*
 *	Calculates a date from a string (generally, an argument string)
 *	Copyright (c) 2003 Pertimm by Patrick Constant
 *	Dev : December 2003
 *	Version 1.0
*/
#include <loggen.h>


#define DOgMaxDateElement   3

struct og_date_element {
  int is; char s[1024];
  };

struct og_month {
  int is; char *s;
  };

static struct og_month month_en[] = {
  {2,"ja"},  {1,"f"},   {3,"mar"},
  {2,"ap"},  {3,"may"}, {3,"jun"},
  {3,"jul"}, {2,"au"},  {1,"s"},
  {1,"o"},   {1,"n"},   {1,"d"},
  {0,""}
  };

static struct og_month month_fr[] = {
  {2,"ja"},  {1,"f"},   {3,"mar"},
  {2,"av"},  {3,"mai"}, {4,"juin"},
  {4,"juil"}, {2,"ao"},  {1,"s"},
  {1,"o"},   {1,"n"},   {1,"d"},
  {0,""}
  };

static int IsNumber(char *);
static int IsMonth(int, int, char *);





/*
 *  Interprets format of the date. Format of the date depends upon the language.
 *  interpretation is as follows:
 *    one element: 
 *      a number n = year
 *      a string s = name of the month
 *    two elements (choice between day and year depends upon value of n): 
 *      n n: (in english month day/year, otherwise day/year month
 *      n s: day/year month
 *      s n: month day/year
 *    three elements: 
 *      n n n: (in english month day year, otherwise day month year)
 *      n s n: day month year
 *      s n n: month day year
 *    all other solutions are not accepted.
 *    returns 1 if date has correct format and 0 otherwise.
*/

PUBLIC(int) OgArgDate(int language, char *raw_date, struct og_date *date)
{
int i,c,end=0,ielem=0,state=1,month;
struct og_date_element elem[DOgMaxDateElement];
int n[DOgMaxDateElement], m[DOgMaxDateElement];
int imonth,iday;

memset(date,0,sizeof(struct og_date));
memset(elem,0,DOgMaxDateElement*sizeof(struct og_date_element));
memset(n,0,DOgMaxDateElement*sizeof(int));
memset(m,0,DOgMaxDateElement*sizeof(int));

for (i=0; !end; i++) {
  if (raw_date[i]==0) { end=1; c=' '; }
  else c = raw_date[i];
  switch (state) {
    case 1:
      if (PcIsalnum(c)) {
        elem[ielem].s[elem[ielem].is++]=c;
        state=2;
        }
      break;
    case 2:
      if (PcIsalnum(c)) {
        elem[ielem].s[elem[ielem].is++]=c;
        }
      else {
        elem[ielem].s[elem[ielem].is]=0;
        ielem++; if (ielem>=DOgMaxDateElement) goto endCreateRequestDate1;
        state=1;
        }   
      break;
    }
  }

endCreateRequestDate1:

/* Since this function is used to analyse dates in tags
 * there is no reason to use current date.
if (1 <= ielem && ielem <= 2) {
  time_t ltime; struct tm *gmt; 
  time(&ltime); gmt = gmtime(&ltime);
  date->mday=gmt->tm_mday; date->month=gmt->tm_mon; date->iyear=1900+gmt->tm_year;
  }
*/

for (i=0; i<ielem; i++) {
  if (IsNumber(elem[i].s)) n[i]=atoi(elem[i].s);
  else if ((month=IsMonth(language,elem[i].is,elem[i].s))) m[i]=month;
  }

if (ielem==1) {
  if (n[0]) {
    date->iyear = n[0]; return(1);
    }
  else if (m[0]) {
    date->mday = 1; date->month = m[0]; return(1);
    }
  }
else if (ielem==2) {
  if (n[0] && n[1]) {
    if (language==DOgLangEN) { imonth=0; iday=1; }
    else { imonth=1; iday=0; }
    date->month = n[imonth];
    if (1 <= date->month && date->month <= 12) {
      date->mday = n[iday];
      if (1 <= date->mday && date->mday <= 31) return(1);
      else { date->iyear = date->mday; return(1); }
      }
    }
  else if (n[0] && m[1]) {
    date->mday = n[0]; date->month=m[1];
    if (1 <= date->mday && date->mday <= 31) return(1);
    else { date->iyear = date->mday; return(1); }
    }
  else if (m[0] && n[1]) {
    date->mday = n[1]; date->month=m[0];
    if (1 <= date->mday && date->mday <= 31) return(1);
    else { date->iyear = date->mday; return(1); }
    }
  }
else if (ielem==3) {
  if (n[0] && n[1] && n[2]) {
    date->iyear=n[2];
    if (language==DOgLangEN) { imonth=0; iday=1; }
    else { imonth=1; iday=0; }
    date->month = n[imonth];
    if (1 <= date->month && date->month <= 12) {
      date->mday = n[iday];
      if (1 <= date->mday && date->mday <= 31) return(1);
      }
    }
  else if (n[0] && m[1]) {
    date->mday = n[0]; date->month=m[1]; date->iyear=n[2];
    if (1 <= date->mday && date->mday <= 31) return(1);
    }
  else if (m[0] && n[1]) {
    date->mday = n[1]; date->month=m[0]; date->iyear=n[2];
    if (1 <= date->mday && date->mday <= 31) return(1);
    }
  }

return(0);
}




static int IsNumber(char *s)
{
int i;
for (i=0; s[i]; i++) {
  if (!PcIsdigit(s[i])) return(0);
  }
return(1);
}




static int IsMonth(int language, int is, char *s)
{
int i;
struct og_month *month=month_en;

if (language==DOgLangNil) language=DOgLangEN;

switch (language) {
  case DOgLangEN: month=month_en; break;
  case DOgLangFR: month=month_fr; break;
  }
for (i=0; month[i].is; i++) {
  if (is < month[i].is) continue;
  if (memcmp(s,month[i].s,month[i].is)) continue;
  return(i+1);
  }
return(0);
}







