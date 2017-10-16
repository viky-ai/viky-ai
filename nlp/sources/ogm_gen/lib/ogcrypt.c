/*
 *	Crypting and decrypting functions
 *	Copyright (c) 2002 Pertimm, Inc by Patrick Constant
 *	Dev : July 2002
 *	Version 1.0
*/
#include <loggen.h>


#define DOgCrcIncrement   11
#define DOgMaxCrypted     0x1000 /* 4k */


static int OgCrc(int);




PUBLIC(int) OgCryptMimeEncode(int ibase, unsigned char *base, int *iencoded, unsigned char *encoded, int ipassword, unsigned char *password)
{
char erreur[DPcSzErr];
unsigned char *crypted;
unsigned char ccrypted[DOgMaxCrypted];
if (ibase < DOgMaxCrypted) crypted=ccrypted;
else {
  IFn(crypted=(unsigned char *)malloc(ibase+10)) {
    DPcSprintf(erreur,"OgMimeEncode: impossible to malloc %d bytes",ibase);
    PcErr(-1,erreur); DPcErr;
    }
  }
IFE(OgCrypt(ibase,base,crypted,ipassword,password));
IFE(OgMimeEncode(ibase,crypted,iencoded,encoded));
DONE;
}





PUBLIC(int) OgCryptMimeDecode(int iencoded, unsigned char *encoded, int *ibase, unsigned char *base, int ipassword, unsigned char *password)
{
IFE(OgMimeDecode(iencoded,encoded,ibase,base));
IFE(OgCrypt(*ibase,base,base,ipassword,password));
DONE;
}





/*
 *  This works for both encryption and decryption algorithm
 *  See description in ogmios/doc/internal_documentation/cryptage.pdf
 *  DOgCrcIncrement is normally 7 but can be any prime number.
*/

PUBLIC(int) OgCrypt(int length, unsigned char *in, unsigned char *out, int ipassword, unsigned char *password)
{
int i, masque, masque_precedent;

/** Encrypting the configuration string **/
masque_precedent = 0;
for (i=0; i<length; i++) {
  masque = OgCrc(masque_precedent) ^ password[i%ipassword];
  out[i] = masque ^ in[i];
  masque_precedent = masque;
  }
DONE;
}




static int OgCrc(int n)
{
return(((n*DOgCrcIncrement)%0x100));
}


