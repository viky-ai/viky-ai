/*
 *	All generic functions for strings
 *	Copyright (c) 2003,2004,2006 by Patrick Constant
 *	Dev : December 2003, January 2004, August 2006
 *	Version 1.3
*/
#include <loggen.h>



/*
 *  Removes space chars at beginning and end of string
 *  and puts the result into 'r'. 'r' can be equal to 's'.
*/
int OgTrimString(unsigned char *s, unsigned char *r)
{
int i,j,c,state=1;
for (i=j=0; s[i]; i++) {
  c=s[i];
  switch (state) {
    case 1:
      if (!isspace(c)) {
        r[j++]=c;
        state=2;
        }
      break;
    case 2:
      r[j++]=c;
      break;
    }
  }
r[j]=0;
for (i=j-1; i>=0; i--) {
  if (!isspace(r[i])) { r[i+1]=0; break; }
  }

DONE;
}




/*
 *  Removes space chars at beginning and end of string
 *  and puts the result into 'r'. 'r' can be equal to 's'.
*/
int OgTrimStringChar(unsigned char *s, unsigned char *r, int trimc)
{
int i,j,c,state=1;
for (i=j=0; s[i]; i++) {
  c=s[i];
  switch (state) {
    case 1:
      if (c!=trimc) {
        r[j++]=c;
        state=2;
        }
      break;
    case 2:
      r[j++]=c;
      break;
    }
  }
r[j]=0;
for (i=j-1; i>=0; i--) {
  if (r[i]!=trimc) { r[i+1]=0; break; }
  }

DONE;
}






/*
 *  Removes space chars at beginning and end of unicode string
 *  and puts the result into 'r'. 'r' can be equal to 's'.
*/
int OgTrimUnicode(int is, unsigned char *s, int *pir, unsigned char *r)
{
int i,j,c,state=1;
for (i=j=0; i<is; i+=2) {
  c=(s[i]<<8)+s[i+1];
  switch (state) {
    case 1:
      if (!PcIsspace(c)) {
        r[j++]=s[i]; r[j++]=s[i+1];
        state=2;
        }
      break;
    case 2:
      r[j++]=s[i]; r[j++]=s[i+1];
      break;
    }
  }
for (i=j-2; i>=0; i-=2) {
  c=(s[i]<<8)+s[i+1];
  if (!PcIsspace(c)) { j=i+2; break; }
  }
*pir=j;
DONE;
}







/*
 *  Removes specified char at beginning and end of unicode string
 *  and puts the result into 'r'. 'r' can be equal to 's'.
*/
int OgTrimUnicodeChar(int is, unsigned char *s, int *pir, unsigned char *r, int trimc)
{
int i,j,c,state=1;
for (i=j=0; i<is; i+=2) {
  c=(s[i]<<8)+s[i+1];
  switch (state) {
    case 1:
      if (c!=trimc) {
        r[j++]=s[i]; r[j++]=s[i+1];
        state=2;
        }
      break;
    case 2:
      r[j++]=s[i]; r[j++]=s[i+1];
      break;
    }
  }
for (i=j-2; i>=0; i-=2) {
  c=(s[i]<<8)+s[i+1];
  if (c!=trimc) { j=i+2; break; }
  }
*pir=j;
DONE;
}







PUBLIC(int) OgReverseString(unsigned char *string1, unsigned char *string2)
{
int length,length2,i;
char buffer;

if(string1!=string2) strcpy(string2,string1);

length=strlen(string2);
length2=length/2;
for(i=0;i<length2;i++) {
  buffer=string2[length-1-i];
  string2[length-1-i]=string2[i];
  string2[i]=buffer;
  }

DONE;
}





/*
 * Returns true if two zones overlap.
 * A zone can be anything that has a start position and a length : strings, buffers, ...
 */
PUBLIC(int) OgZoneOverlap(int start1, int length1, int start2, int length2)
{
int end1 = start1 + length1;
int end2 = start2 + length2;

if (start1 <= start2 && start2 < end1) return(1);
if (start2 <= start1 && start1 < end2) return(1);

return(0);
}


/**
 * Copy a fixed length string string to a fixed buffer length.<br>
 * If the buffer is too small to store the src_string, src_string will be truncated.
 *
 * @param dest_buffer destination buffer
 * @param dest_buffer_size destination buffer size
 * @param src_string source of the string copy
 * @param src_string_length length of the source string
 * @return nb char copied
 */
inline PUBLIC(int) OgStrCpySized(unsigned char *dest_buffer, int dest_buffer_size, const unsigned char *src_string,
    int src_string_length)
{
  // compute src length to copy
  int result = src_string_length;
  if (result > dest_buffer_size)
  {
    result = dest_buffer_size;
  }

  // copy
  memcpy(dest_buffer, src_string, result);

  return result;
}

/**
 * Copy a fixed length string string to a fixed buffer length.<br>
 * If the buffer is too small to store the src_string, src_string will be truncated.<br>
 * The dest buffer will be always terminated by \0\0 (unicode support).
 *
 * @param dest_buffer destination buffer
 * @param dest_buffer_size destination buffer size
 * @param src_string source of the string copy
 * @param src_string_length length of the source string
 * @return nb char copied
 */
inline PUBLIC(int) OgStrCpySizedTerm(unsigned char *dest_buffer, int dest_buffer_size, const unsigned char *src_string,
    int src_string_length)
{
  // compute src length to copy
  int result = src_string_length;
  if (result > (dest_buffer_size - 2))
  {
    result = dest_buffer_size - 2;
  }

  // copy
  memcpy(dest_buffer, src_string, result);

  // terminate string
  dest_buffer[result + 1] = 0;

  // terminate unicode string
  dest_buffer[result + 2] = 0;

  return result;
}

