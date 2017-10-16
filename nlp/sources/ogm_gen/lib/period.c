/*
 *  Handling of periods.
 *	Copyright (c) 2003-2007 Pertimm by Patrick Constant
 *	Dev : November 2003, March 2004, December 2007
 *	Version 1.2
*/
#include <loggen.h>




PUBLIC(int) OgGetPeriod(char *s, struct og_period *period)
{
int length,pe;
length = strlen(s);
pe = PcTolower(s[length-1]);
switch (pe) {
  case DOgPeriodTypeSecond:
    period->nb_seconds = 1;
    period->type = pe;
    break;
  case DOgPeriodTypeMinute:
    period->nb_seconds = 60;
    period->type = pe;
    break;
  case DOgPeriodTypeHour:
    period->nb_seconds = 3600;
    period->type = pe;
    break;
  case DOgPeriodTypeDay:
    period->nb_seconds = 3600*24;
    period->type = pe;
    break;
  case DOgPeriodTypeWeek:
    period->nb_seconds = 3600*24*7;
    period->type = pe;
    break;
  case DOgPeriodTypeMonth:
    /** average value of the month **/
    period->nb_seconds = 3600*24*31;
    period->type = pe;
    break;
  case DOgPeriodTypeYear:
    /** average value of the year **/
    period->nb_seconds = 3600*24*365;
    period->type = pe;
    break;
  default:
    period->nb_seconds = 3600*24;
    period->type = DOgPeriodTypeDay;
    break;
  }
period->value=atoi(s);
period->nb_seconds *= period->value;
DONE;
}





/*
 *  Calculates period starting/ending date and period number 
 *  from a given date and period.
 *  first_span_period can be zero, in that case we do not calculate period number.
 *  otherwise first_span_period contains the first (most recent) period calculated 
 *  by this function.
*/

PUBLIC(int) OgGetSpanPeriod(struct og_span_period *first_span_period, time_t date, struct og_period *period, struct og_span_period *span_period)
{
char erreur[DPcSzErr];
struct tm *d, psd,ped;
int nb_seconds,nb_years,nb_months,factor;

memset(span_period,0,sizeof(struct og_span_period));
d=localtime(&date); psd=*d;

switch(period->type) {
  case DOgPeriodTypeSecond:
    psd.tm_isdst=0;
    break;
  case DOgPeriodTypeMinute:
    psd.tm_sec=0;
    psd.tm_isdst=0;
    break;    
  case DOgPeriodTypeHour:
    psd.tm_sec=0;
    psd.tm_min=0;
    psd.tm_isdst=0;
    break;    
  case DOgPeriodTypeDay:
    psd.tm_sec=0;
    psd.tm_min=0;
    psd.tm_hour=0;
    psd.tm_isdst=0;
    break;    
  case DOgPeriodTypeWeek:
    psd.tm_sec=0;
    psd.tm_min=0;
    psd.tm_hour=0;
    psd.tm_wday=0; /* Week starting on sunday */
    psd.tm_isdst=0;
    break;    
  case DOgPeriodTypeMonth:
    psd.tm_sec=0;
    psd.tm_min=0;
    psd.tm_hour=0;
    psd.tm_mday=1;
    psd.tm_isdst=0; 
    break;
  case DOgPeriodTypeYear:
    psd.tm_sec=0;
    psd.tm_min=0;
    psd.tm_hour=0;
    psd.tm_mday=1; 
    psd.tm_mon=0; 
    psd.tm_yday=0;
    psd.tm_isdst=0; 
    break;
  default:
    sprintf(erreur,"OgGetSpanPeriod: bad period type %c",period->type);
    PcErr(-1,erreur); DPcErr;
  }

if ((span_period->starting_date = mktime(&psd))==(unsigned)-1) {
  sprintf(erreur,"OgGetSpanPeriod: mktime error");
  PcErr(-1,erreur); DPcErr;
  }

if (period->type == DOgPeriodTypeWeek && psd.tm_wday>0) {
  span_period->starting_date -= psd.tm_wday*24*3600;
  d=localtime(&span_period->starting_date); psd=*d;
  }

IFn(first_span_period) nb_seconds=0;
else {
  nb_seconds=first_span_period->starting_date - span_period->starting_date;
  if (nb_seconds < 0) {
    sprintf(erreur,"OgGetSpanPeriod: nb_seconds (%d) < 0",nb_seconds);
    PcErr(-1,erreur); DPcErr;
    }
  }

/** Calculating the period number and ending date **/
switch(period->type) {
  case DOgPeriodTypeSecond:
    factor=1; span_period->number = nb_seconds/factor;
    span_period->ending_date = span_period->starting_date + factor;
    break;
  case DOgPeriodTypeMinute:
    factor=60; span_period->number = nb_seconds/factor;
    span_period->ending_date = span_period->starting_date + factor;
    break;    
  case DOgPeriodTypeHour:
    factor=3600; span_period->number = nb_seconds/factor;
    span_period->ending_date = span_period->starting_date + factor;
    break;    
  case DOgPeriodTypeDay:
    factor=3600*24; span_period->number = nb_seconds/factor;
    span_period->ending_date = span_period->starting_date + factor;
    break;    
  case DOgPeriodTypeWeek:
    factor=3600*24*7; span_period->number = nb_seconds/factor;
    span_period->ending_date = span_period->starting_date + factor;
    break;    
  case DOgPeriodTypeMonth:
    IFx(first_span_period) {
      d=localtime(&first_span_period->starting_date);
      nb_years = d->tm_year - psd.tm_year;
      nb_months = d->tm_mon - psd.tm_mon;
      /** nb_months can be negative but the formula is then also correct **/
      span_period->number = nb_years*12 + nb_months;
      }
    ped = psd;
    if (ped.tm_mon < 11) ped.tm_mon++;
    else { ped.tm_mon = 0; ped.tm_year++; }
    if ((span_period->ending_date = mktime(&ped))==(unsigned)-1) {
      sprintf(erreur,"OgGetSpanPeriod: mktime error");
      PcErr(-1,erreur); DPcErr;
      }
    break;
  case DOgPeriodTypeYear:
    IFx(first_span_period) {
      d=localtime(&first_span_period->starting_date);
      nb_years = d->tm_year - psd.tm_year;
      span_period->number = nb_years;
      }
    ped = psd; ped.tm_year++;
    if ((span_period->ending_date = mktime(&ped))==(unsigned)-1) {
      sprintf(erreur,"OgGetSpanPeriod: mktime error");
      PcErr(-1,erreur); DPcErr;
      }
    break;
  default:
    sprintf(erreur,"OgGetSpanPeriod: bad period type %d",period->type);
    PcErr(-1,erreur); DPcErr;
  }

span_period->number /= period->value;

DONE;
}






PUBLIC(int) OgGetEndPeriod(time_t date, struct og_period *period, time_t *end_date)
{
time_t edate;
struct tm gmt, *pgmt;
char erreur[DPcSzErr];
int nb_months;

switch(period->type) {
  case DOgPeriodTypeSecond: edate = date+1; break;
  case DOgPeriodTypeMinute: edate = date+60; break;
  case DOgPeriodTypeHour: edate = date+3600; break;
  case DOgPeriodTypeDay: edate = date+3600*24; break;
  case DOgPeriodTypeWeek: edate = date+3600*24*7; break;
  case DOgPeriodTypeMonth:
    pgmt=gmtime(&date); gmt=*pgmt;
    nb_months = gmt.tm_mon + period->value;
    if (nb_months < 12) gmt.tm_mon=nb_months;
    else {
      gmt.tm_year=gmt.tm_year+nb_months/12;
      gmt.tm_mon=nb_months%12;
      }
    if ((edate = mktime(&gmt))==(time_t)-1) {
      edate=0x7fffffff; /* GMT: Tue, 19 Jan 2038 03:14:07 */
      }
    
    break;
  case DOgPeriodTypeYear:
    pgmt=gmtime(&date); gmt=*pgmt;
    gmt.tm_year+=period->value;
    if ((edate = mktime(&gmt))==(time_t)-1) {
      edate=0x7fffffff; /* GMT: Tue, 19 Jan 2038 03:14:07 */
      }
    break;
  default:
    sprintf(erreur,"OgGetSpanPeriod: bad period type %d",period->type);
    PcErr(-1,erreur); DPcErr;
  }
*end_date = edate;
DONE;
}


