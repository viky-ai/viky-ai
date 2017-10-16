/*
 *  Getting hard drive serial number
 *  Copyright (c) 2005 Pertimm
 *  Dev : January 2005
 *  Version 1.0
*/
#include <loggen.h>



/** defined in diskid32.c which is public domain. **/
int GetHardDriveSerialNumber(char *);




PUBLIC(int) OgGetHardDriveSerialNumber(char *serial_number)
{
GetHardDriveSerialNumber(serial_number);
DONE;
}


