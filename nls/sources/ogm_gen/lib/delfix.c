/*
 *	List of Prefixes to be deleted from words. 
 *	Copyright (c) 2001 Pertimm, Inc. by Patrick Constant
 *	Dev : January 2002
 *	Version 1.0
*/
#include <loggen.h>


PUBLIC(int) OgDelPrefixInit(struct og_del_prefix **pdelfix)
{
int i = 0;
unsigned short *s;
static int initialized = 0;
static struct og_del_prefix delfix[20];

*pdelfix = delfix;

if (initialized) DONE;

delfix[i].ich = 2; s = delfix[i++].ch; s[0]='d'; s[1]='\''; s[2]=0; 
delfix[i].ich = 2; s = delfix[i++].ch; s[0]='j'; s[1]='\''; s[2]=0; 
delfix[i].ich = 2; s = delfix[i++].ch; s[0]='l'; s[1]='\''; s[2]=0; 
delfix[i].ich = 2; s = delfix[i++].ch; s[0]='m'; s[1]='\''; s[2]=0; 
delfix[i].ich = 2; s = delfix[i++].ch; s[0]='s'; s[1]='\''; s[2]=0; 
delfix[i].ich = 2; s = delfix[i++].ch; s[0]='t'; s[1]='\''; s[2]=0; 
delfix[i].ich = 3; s = delfix[i++].ch; s[0]='q'; s[1]='u'; s[2]='\''; s[3]=0;

DONE;
}



PUBLIC(int) OgDelPrefixEqual(struct og_del_prefix *delfix, short unsigned int *word)
{
int i;
for (i=0; i<delfix->ich; i++) {
  if (delfix->ch[i]!=PcTolower(word[i])) return(0);
  }
return(1);
}

