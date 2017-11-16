/*
 *  Parsing function leaves
 *  Copyright (c) 2006-2008 Pertimm by Patrick Constant
 *  Dev : October 2006, March 2007, May 2008
 *  Version 1.2
*/
#include "ogm_rqp.h"


//#define DEVERMINE

static int AddArg(struct og_ctrl_rqp *, int, int *, struct og_rqp_arg *,
                    unsigned char *, int, int);


/*
 * function syntax is typically:
 *   func(a,"un mot(joli)")
 *   func(a,"un \"mot\"")
 * arg[0] is name of function.
*/

PUBLIC(int) OgRqpParseFunction(void *handle, int is, unsigned char *s, int arg_size, int *parg_length, struct og_rqp_arg *arg)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
int i,c,end=0,state=1,start=0;
int parenthesis_level=0;
int in_dquote=0,added;

#ifdef DEVERMINE
{
int ibuffer; char buffer[DPcPathSize];
IFE(OgUniToCp(is, s
  ,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
  , "OgRqpParseFunction: working on '%s'", buffer);
}
#endif

*parg_length=0;

int cbefore = 0;
for (i=0; !end; i+=2) {
  cbefore = c;
  if (i>=is) { c=','; end=1; }
  else c=(s[i]<<8)+s[i+1];
  switch (state) {
    /** before function name **/
    case 1:
      if (!PcIsspace(c) && c!=';' && c!='"') {
        start=i; state=2;
        }
      break;
    /** in function name **/
    case 2:
      if (c=='(' && parenthesis_level==0) {
        parenthesis_level++;
        IFE(added=AddArg(ctrl_rqp,arg_size,parg_length,arg,s,start,i-start));
        if (!added) goto endOgRqpParseFunction;
        state=3;
        }
      break;
    /** before argument **/
    case 3:
      if (!PcIsspace(c)) {
        if (c==')' || c==',') {
          if (c==')') parenthesis_level--;
          IFE(added=AddArg(ctrl_rqp,arg_size,parg_length,arg,s,i,0));
          if (!added) goto endOgRqpParseFunction;
          if (c==',') state=3;
          else goto endOgRqpParseFunction;
          break;
          }
        if (c=='"') in_dquote=1; else in_dquote=0;
        start=i; state=4;
        }
      break;
    /** in argument **/
    case 4:
      if (c==')') { if (!in_dquote) parenthesis_level--; }
      else if (c=='(') { if (!in_dquote) parenthesis_level++; }
      if ((c==')' && parenthesis_level==0) || (c==',' && parenthesis_level==1) ) {
        if (!in_dquote) {
          IFE(added=AddArg(ctrl_rqp,arg_size,parg_length,arg,s,start,i-start));
          if (!added) goto endOgRqpParseFunction;
          if (c==',') state=3;
          else goto endOgRqpParseFunction;
          }
        }
      else if (cbefore != '\\' && c=='"') {
        if (in_dquote) {
          in_dquote=0;
          }
        else in_dquote=1;
        }
      break;
    }
  }

endOgRqpParseFunction:

DONE;
}





static int AddArg(struct og_ctrl_rqp *ctrl_rqp, int arg_size, int *parg_length, struct og_rqp_arg *arg, unsigned char *s, int start, int length)
{
int c1,c2,last,offset=0, arg_length=*parg_length;

if (arg_length >= arg_size) return(0);

if (length > 0) {
  c1=(s[start]<<8)+s[start+1]; last=start+length-2;
  c2=(s[last]<<8)+s[last+1];
  if (c1=='"' && c2=='"') offset=2;
  }

arg[arg_length].length=length-offset*2;
arg[arg_length].start=start+offset;

#ifdef DEVERMINE
{
int ibuffer; char buffer[DPcPathSize];
IFE(OgUniToCp(arg[arg_length].length,s+arg[arg_length].start
  ,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
  , "AddArg: adding argument '%s'", buffer);
}
#endif

arg_length++; *parg_length=arg_length;

return(1);
}




