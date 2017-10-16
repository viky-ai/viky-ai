/*
 *	This function encapsulates rand function
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev : January 2008
 *	Version 1.0
*/
#include <loggen.h>




PUBLIC(int) OgSrand(unsigned int seed)
{
srand(seed);
DONE;
}




PUBLIC(int) OgRand(unsigned int range, unsigned int *pvalue)
{
ogint64_t v; unsigned sv;
unsigned random_number;
/** Windows uses 0x7fff as RAND_MAX while Linux uses 7fffffff **/
if (RAND_MAX <= 0xffff) random_number=rand()*rand();
else random_number=rand();
v=range; v*=random_number; 
if (RAND_MAX <= 0xffff) v/=RAND_MAX; v/=RAND_MAX;
sv=(size_t)v;
*pvalue=sv;
DONE;
}




