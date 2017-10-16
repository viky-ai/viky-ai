/*
 *  Encapsulation of Sleep function
 *  Copyright (c) 1997-2000  Ogmios by M'hammed El Ajli and Patrick Constant
 *  Dev : October 1997, January 1998, January 2000
 *  Version 1.2
*/
#include <ctype.h>
#include <loggen.h>



static int OgAtoT1(int, char *, struct tm *);



/*
 *  Returns the number of milli-seconds from start of process.
 *  The value of the nanoseconds field must be in the range 0 to 999 999 999.
*/

#if (DPcSystem == DPcSystemUnix)
#include <sys/times.h>

#if (DPcArch == DPcArchlinux)

#if 1
PUBLIC(int) OgSleep(int nb_milliseconds)
{
struct timespec req, rem;
req.tv_sec = nb_milliseconds / 1000;
req.tv_nsec = (nb_milliseconds % 1000) * 1000000;
nanosleep(&req,&rem);
DONE;
}

#else
/* This is an alternative way of sleep which is a bit more precise
 * than nanosleep, it can also be used when nanosleep does not exist */
PUBLIC(int) OgSleep(int nb_milliseconds)
{
struct timeval tv;
tv.tv_sec = nb_milliseconds/1000;
tv.tv_usec = (nb_milliseconds%1000) * 1000;
select(0, NULL, NULL, NULL, &tv);
DONE;
}
#endif

PUBLIC(int) OgNanoSleep(ogint64_t nb_nanoseconds)
{
struct timespec req, rem;
req.tv_sec = nb_nanoseconds / 1000000000;
req.tv_nsec = (nb_nanoseconds % 1000000000);
nanosleep(&req,&rem);
DONE;
}


#else
#if (DPcArch == DPcArchsolaris)

PUBLIC(int) OgSleep(int nb_milliseconds)
{
int nb_seconds, rest_milliseconds;
int start,i,dummy;
nb_seconds = nb_milliseconds / 1000;
rest_milliseconds = nb_milliseconds % 1000;
if (nb_seconds) sleep(nb_seconds);
start = OgClock();
do { for (i=0; i<100000; i++) dummy=1;}
while (OgClock() - start < rest_milliseconds);
DONE;
}

#else
#if (DPcArch == DPcArchtru64)

PUBLIC(int) OgSleep(int nb_milliseconds)
{
int nb_seconds, rest_milliseconds;
int start,i,dummy;
nb_seconds = nb_milliseconds / 1000;
rest_milliseconds = nb_milliseconds % 1000;
if (nb_seconds) sleep(nb_seconds);
start = OgClock();
do { for (i=0; i<100000; i++) dummy=1;}
while (OgClock() - start < rest_milliseconds);
DONE;
}

#else
#if (DPcArch == DPcArchaix)

PUBLIC(int) OgSleep(int nb_milliseconds)
{
int nb_seconds, rest_milliseconds;
int start,i,dummy;
nb_seconds = nb_milliseconds / 1000;
rest_milliseconds = nb_milliseconds % 1000;
if (nb_seconds) sleep(nb_seconds);
start = OgClock();
do { for (i=0; i<100000; i++) dummy=1;}
while (OgClock() - start < rest_milliseconds);
DONE;
}

#else

PUBLIC(int) OgSleep(int nb_milliseconds)
{
int nb_seconds, rest_milliseconds;
int start,i,dummy;
nb_seconds = nb_milliseconds / 1000;
rest_milliseconds = nb_milliseconds % 1000;
if (nb_seconds) sleep(nb_seconds);
start = OgClock();
do { for (i=0; i<100000; i++) dummy=1;}
while (OgClock() - start < rest_milliseconds);
DONE;
}

#endif
#endif
#endif
#endif





/*
 *  To the contrary of Windows, clock() returns
 *  process running time, while on Windows it returns
 *  elapsed time. So we must use 'times'. At least on
 *  Linux, the value returned must be multiplied by 10
 *  to obtain a milli-seconds value.
*/

PUBLIC(int) OgClock(void)
{
struct tms elapsed;
return(10*times(&elapsed));
}


PUBLIC(ogint64_t) OgMicroClock(void)
{
ogint64_t t;
struct timeval tv;
gettimeofday(&tv,0);
t = tv.tv_sec; t *= 1000000;
t += tv.tv_usec;
return(t);
}




#else
#if (DPcSystem == DPcSystemWin32)
#include <windows.h>


PUBLIC(int) OgSleep(int nb_milliseconds)
{
Sleep(nb_milliseconds);
DONE;
}


PUBLIC(int) OgClock()
{
return(clock());
}


#ifndef EPOCHFILETIME
#define EPOCHFILETIME (116444736000000000i64)
#endif

PUBLIC(ogint64_t) OgMicroClock()
{
ogint64_t t;
FILETIME ft;
LARGE_INTEGER li;

GetSystemTimeAsFileTime(&ft);
li.LowPart  = ft.dwLowDateTime;
li.HighPart = ft.dwHighDateTime;
t  = li.QuadPart;       /* In 100-nanosecond intervals */
t -= EPOCHFILETIME;     /* Offset to the Epoch time */
t /= 10;                /* In microseconds */
return(t);
}


#endif
#endif



PUBLIC(int) OgMilliClock(void)
{
return((int)(OgMicroClock()/1000));
}



PUBLIC(int) OgTtoA(time_t t, int is, char *s)
{
struct tm *t_tm;
t_tm=localtime(&t);
if (t_tm == 0) return(0);
return(strftime(s, is, "%Y/%m/%d/%H/%M/%S", t_tm));
}






PUBLIC(int) OgAtoT(int is, char *s, unsigned int *t)
{
struct tm ctm;
int i,inumber=0,state;
char erreur[DPcSzErr];
char number[100];
int nth = 0;

/** Default values are <year>/01/01/00/00/00 **/
memset(&ctm,0,sizeof(struct tm));
ctm.tm_mday = 1;

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
        IFE(OgAtoT1(nth++,number,&ctm));
        state = 3;
        }
      else if (c == '/') {
        number[inumber]=0;
        IFE(OgAtoT1(nth++,number,&ctm));
        state = 1;
        }
      else number[inumber++] = c;
      break;
    case 3:
      if (i==is || isspace(c));
      else if (c == '/') state = 1;
      else {
        sprintf(erreur,"OgAtoT: malformed date '%.*s'",is,s);
        PcErr(-1,erreur); DPcErr;
        }
      break;
    }
  }
ctm.tm_isdst = -1;
if ((*t = mktime(&ctm))==(unsigned)-1) {
  sprintf(erreur,"OgAtoT: mktime error on '%.*s'",is,s);
  PcErr(-1,erreur); DPcErr;
  }

DONE;
}



/*
 * tm_hour  Hours since midnight (0-23)
 * tm_isdst  Positive if daylight saving time is in effect;
 *   0 if daylight saving time is not in effect;
 *   negative if status of daylight saving time is unknown.
 *   The C run-time library assumes the United States�s rules for implementing
 *   the calculation of Daylight Saving Time (DST).
 * tm_mday  Day of month (1-31)
 * tm_min  Minutes after hour (0-59)
 * tm_mon  Month (0-11; January = 0)
 * tm_sec  Seconds after minute (0-59)
 * tm_wday  Day of week (0-6; Sunday = 0)
 * tm_yday  Day of year (0-365; January 1 = 0)
 * tm_year  Year (current year minus 1900)
*/

/*
 *  Fills-in a field of struct tm. Interpretation of the date is limited
 *  from 01/01/1970 00:00:00 to 19/01/2038 04:14:07
*/

static int OgAtoT1(int nth, char *number, struct tm *tm)
{
int n,num;
char erreur[DPcSzErr];

switch(nth) {

  case 0:
    n = atoi(number);
    if (0 <= n && n < 70) num = 100 + n;
    else if (70 <= n && n <= 99) num = n;
    else if (1970 <= n) num = n - 1900;
    else  {
      sprintf(erreur,"OgAtoT1: bad year '%s'",number);
      PcErr(-1,erreur); DPcErr;
      }
    tm->tm_year = num;
    break;

  case 1:
    n = atoi(number);
    if (1 <= n && n <= 12) num = n - 1;
    else  {
      sprintf(erreur,"OgAtoT1: bad month '%s'",number);
      PcErr(-1,erreur); DPcErr;
      }
    tm->tm_mon = num;
    break;

  case 2:
    n = atoi(number);
    if (1 <= n && n <= 31) num = n;
    else  {
      sprintf(erreur,"OgAtoT1: bad day '%s'",number);
      PcErr(-1,erreur); DPcErr;
      }
    tm->tm_mday = num;
    break;

  case 3:
    n = atoi(number);
    if (0 <= n && n <= 23) num = n;
    else  {
      sprintf(erreur,"OgAtoT1: bad hour '%s'",number);
      PcErr(-1,erreur); DPcErr;
      }
    tm->tm_hour = num;
    break;

  case 4:
    n = atoi(number);
    if (0 <= n && n <= 59) num = n;
    else  {
      sprintf(erreur,"OgAtoT1: bad minute '%s'",number);
      PcErr(-1,erreur); DPcErr;
      }
    tm->tm_min = num;
    break;

  case 5:
    n = atoi(number);
    if (0 <= n && n <= 59) num = n;
    else  {
      sprintf(erreur,"OgAtoT1: bad second '%s'",number);
      PcErr(-1,erreur); DPcErr;
      }
    tm->tm_sec = num;
    break;

  default:
    sprintf(erreur,"OgAtoT1: too many numbers at '%s'",number);
    PcErr(-1,erreur); DPcErr;

  }
DONE;
}



PUBLIC(char *) OgGmtime(const time_t *timet)
{
return(asctime(gmtime(timet)));
}

