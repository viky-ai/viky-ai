/*
 *	This function gives automatic description of endian order.
 *	Copyright (c) 2000-2005 Pertimm by Patrick Constant
 *	Dev : July 2000, May,July 2005
 *	Version 1.1
*/
#include <loggen.h>


/*
 *  Returns 1 if the system is big endian and 0 otherwise
 *  Intel processors (x86 and Pentium) are little-endian 
 *  while Motorola processors (680x0) are big-endian. 
 *  May 24th 2005: This function should be called OgLittleEndian.
 *  see http://www.webopedia.com/TERM/b/big_endian.html
 *  There is nothing to change, except the name of the function.
 *  We provide the right name for new code, and over the time
 *  the OgBigEndian should disappear.
*/

PUBLIC(int) OgLittleEndian(void)
{
return(OgBigEndian());
}



PUBLIC(int) OgBigEndian(void)
{
unsigned number = 0xffffffee;
unsigned char string[5];
memcpy(string,&number,4);
/*
 * if (string[3] == 0xee) return(0);
 * else return(1);
*/
if (string[0] == 0xee) return(1);
else return(0);
}


