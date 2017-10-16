/*
 *	Reads a tag, and calls a function with name attribute and value
 *	Copyright (c) 2004-2008 Pertimm by Patrick Constant
 *	Dev : April 2004, March 2008
 *	Version 1.1
*/
#include <loggen.h>


/*
 * Function func is called at name, at every name attr and at every name attr=value
 * whether name has attributes or attributes have values or not.
 * Same as OgReadTag, but unicode based
*/

PUBLIC(int) OgReadTagUni(int is, unsigned char *s, int offset, int (*func) (void *, struct og_read_tag *), void *ptr)
{
unsigned char name[DOgReadTagUniDataSize+9];
unsigned char attr[DOgReadTagUniDataSize+9];
unsigned char value[DOgReadTagUniDataSize+9];
int i,c,state,end,start_value=0;
struct og_read_tag crt, *rt=&crt;
memset(rt,0,sizeof(struct og_read_tag));
rt->name=name; rt->attr=attr; rt->value=value;

attr[0]=0; attr[1]=0; value[0]=0; value[1]=0;
state=1; end=0;

for (i=0; !end; i+=2) {
  if (i>=is) { end=1; c=' '; }
  else c = (s[i]<<8)+s[i+1];
  switch (state) {
    /* tag name */
    case 1:
      if (PcIsspace(c) || c=='>') {
        if (s[0]==0 && s[1]=='/') rt->closing_tag=1; else rt->closing_tag=0;
        rt->iname=i-rt->closing_tag*2; if (rt->iname>DOgReadTagUniDataSize-2) rt->iname=DOgReadTagUniDataSize-2;
        memcpy(name,s+rt->closing_tag*2,rt->iname); name[rt->iname]=0;
        IFE((*func)(ptr,rt)); state=2;
        }
      break;
    /* space before attr */
    case 2:
      if (PcIsspace(c)) state=2;
      else { rt->iattr=0; attr[rt->iattr++]=(c>>8); attr[rt->iattr++]=(c&0xff); rt->ivalue=0; value[0]=0; state=3; }
      break;
    /* attr */
    case 3:
      if (PcIsspace(c)) { attr[rt->iattr]=0; IFE((*func)(ptr,rt)); state=4; }
      else if (c=='=') { attr[rt->iattr]=0; IFE((*func)(ptr,rt)); state=5; }
      else { if (rt->iattr < DOgReadTagUniDataSize-1) { attr[rt->iattr++]=(c>>8); attr[rt->iattr++]=(c&0xff); } state=3; }
      break;
    /* space after attr */
    case 4:
      if (PcIsspace(c)) state=4;
      else if (c=='=') state=5;
      else { 
        rt->iattr=0; attr[rt->iattr++]=(c>>8); attr[rt->iattr++]=(c&0xff); rt->ivalue=0; value[0]=0; state=3; 
        }
      break;
    /* '=' after attr */
    case 5:
      if (PcIsspace(c)) state=5;
      else if (c=='\'') { rt->ivalue=0; value[0]=0; start_value=i+1; state=6; }
      else if (c=='"') { rt->ivalue=0; value[0]=0; start_value=i+1; state=7; }
      else { rt->ivalue=0; value[rt->ivalue++]=(c>>8); value[rt->ivalue++]=(c&0xff); start_value=i; state=8; }
      break;
    case 6:
      if (c=='\'') {
        value[rt->ivalue]=0; OgTrimUnicode(rt->ivalue,value,&rt->ivalue,value); rt->offset=offset+start_value;
        IFE((*func)(ptr,rt)); state=2;
        }
      else { if (rt->ivalue < DOgReadTagUniDataSize-1) { value[rt->ivalue++]=(c>>8); value[rt->ivalue++]=(c&0xff); } }
      break;
    case 7:
      if (c=='"') {
        value[rt->ivalue]=0; OgTrimUnicode(rt->ivalue,value,&rt->ivalue,value); rt->offset=offset+start_value;
        IFE((*func)(ptr,rt)); state=2;
        }
      else { if (rt->ivalue < DOgReadTagUniDataSize-1) { value[rt->ivalue++]=(c>>8); value[rt->ivalue++]=(c&0xff); } }
      break;
    
    case 8:
      if (PcIsspace(c)) {
        value[rt->ivalue]=0; OgTrimUnicode(rt->ivalue,value,&rt->ivalue,value); rt->offset=offset+start_value;
        IFE((*func)(ptr,rt)); state=2;
        }
      else { if (rt->ivalue < DOgReadTagUniDataSize-1) { value[rt->ivalue++]=(c>>8); value[rt->ivalue++]=(c&0xff); } }
      break;
    }
  }
DONE;
}


