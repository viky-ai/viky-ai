/*
 *	Reads a tag, and calls a function with name attribute and value
 *	Copyright (c) 2004 Pertimm by Patrick Constant
 *	Dev : April 2004
 *	Version 1.0
*/
#include <loggen.h>


/*
 * Function func is called at name, at every name attr and at every name attr=value
 * whether name has attributes or attributes have values or not.
*/

PUBLIC(int) OgReadTag(int is, unsigned char *s, int offset, int (*func) (void *, struct og_read_tag *), void *ptr)
{
unsigned char name[DOgReadTagDataSize+9];
unsigned char attr[DOgReadTagDataSize+9];
unsigned char value[DOgReadTagDataSize+9];
int i,c,state,end,start_value=0;
struct og_read_tag crt, *rt=&crt;
memset(rt,0,sizeof(struct og_read_tag));
rt->name=name; rt->attr=attr; rt->value=value;

attr[0]=0; value[0]=0;
state=1; end=0;

for (i=0; !end; i++) {
  if (i>=is) { end=1; c=' '; }
  else c = s[i];
  switch (state) {
    /* tag name */
    case 1:
      if (isspace(c) || c=='>') {
        if (s[0]=='/') rt->closing_tag=1; else rt->closing_tag=0;
        rt->iname=i-rt->closing_tag; if (rt->iname>DOgReadTagDataSize-1) rt->iname=DOgReadTagDataSize-1;
        memcpy(name,s+rt->closing_tag,rt->iname); name[rt->iname]=0;
        IFE((*func)(ptr,rt)); state=2;
        }
      break;
    /* space before attr */
    case 2:
      if (isspace(c)) state=2;
      else { rt->iattr=0; attr[rt->iattr++]=c; rt->ivalue=0; value[0]=0; state=3; }
      break;
    /* attr */
    case 3:
      if (isspace(c)) { attr[rt->iattr]=0; IFE((*func)(ptr,rt)); state=4; }
      else if (c=='=') { attr[rt->iattr]=0; IFE((*func)(ptr,rt)); state=5; }
      else { if (rt->iattr < DOgReadTagDataSize-1) attr[rt->iattr++]=c; state=3; }
      break;
    /* space after attr */
    case 4:
      if (isspace(c)) state=4;
      else if (c=='=') state=5;
      else { 
        rt->iattr=0; attr[rt->iattr++]=c; rt->ivalue=0; value[0]=0; state=3; 
        }
      break;
    /* '=' after attr */
    case 5:
      if (isspace(c)) state=5;
      else if (c=='\'') { rt->ivalue=0; value[0]=0; start_value=i+1; state=6; }
      else if (c=='"') { rt->ivalue=0; value[0]=0; start_value=i+1; state=7; }
      else { rt->ivalue=0; value[rt->ivalue++]=c; start_value=i; state=8; }
      break;
    case 6:
      if (c=='\'') {
        value[rt->ivalue]=0; OgTrimString(value,value); rt->offset=offset+start_value;
        IFE((*func)(ptr,rt)); state=2;
        }
      else { if (rt->ivalue < DOgReadTagDataSize-1) value[rt->ivalue++]=c; }
      break;
    case 7:
      if (c=='"') {
        value[rt->ivalue]=0; OgTrimString(value,value); rt->offset=offset+start_value;
        IFE((*func)(ptr,rt)); state=2;
        }
      else { if (rt->ivalue < DOgReadTagDataSize-1) value[rt->ivalue++]=c; }
      break;
    
    case 8:
      if (isspace(c)) {
        value[rt->ivalue]=0; OgTrimString(value,value); rt->offset=offset+start_value;
        IFE((*func)(ptr,rt)); state=2;
        }
      else { if (rt->ivalue < DOgReadTagDataSize-1) value[rt->ivalue++]=c; }
      break;
    }
  }
DONE;
}


