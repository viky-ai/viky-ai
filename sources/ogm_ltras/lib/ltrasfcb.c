/*
 *  Flow chart function for Linguistic trf search
 *  Copyright (c) 2009,2010 Pertimm, by Patrick Constant
 *  Dev : November 2009, January 2010
 *  Version 1.1
*/
#include "ogm_ltras.h"


#define DOgLtrasFlowChartMaxNbArgvs 100


static int LtrasFcbSubtree(struct og_ctrl_ltras *ctrl_ltras
  ,struct og_ltra_trfs *input, struct og_ltra_trfs **output,int Irqp_node);
static int LtrasFcbLeaf(struct og_ctrl_ltras *ctrl_ltras
  ,struct og_ltra_trfs *input, struct og_ltra_trfs **output,int Irqp_node);
static int LtrasFcbAnd(struct og_ctrl_ltras *ctrl_ltras
  ,struct og_ltra_trfs *input, struct og_ltra_trfs **output,int Irqp_node);
static int LtrasFcbOr(struct og_ctrl_ltras *ctrl_ltras
  ,struct og_ltra_trfs *input, struct og_ltra_trfs **output,int Irqp_node);




int LtrasFlowChartBoolean(struct og_ctrl_ltras *ctrl_ltras
  , struct og_ltra_trfs *input, struct og_ltra_trfs **output)
{
char erreur[DOgErrorSize];
int Iroot_node;

*output=0;
IF(Iroot_node=OgRqpGetRootNode(ctrl_ltras->hrqp)) {
  /** flowchart is empty, thus we send input **/
  OgErrLast(ctrl_ltras->herr,erreur,0);
  }
else {
  IFE(LtrasFcbSubtree(ctrl_ltras, input, output, Iroot_node));
  }
/** If we have not built any output, we send input **/
IFn(*output) *output=input;

DONE;
}




static int LtrasFcbSubtree(struct og_ctrl_ltras *ctrl_ltras
  ,struct og_ltra_trfs *input, struct og_ltra_trfs **output,int Irqp_node)
{
struct og_rqp_node cnode,*node=&cnode;
char erreur[DOgErrorSize];

IFE(OgRqpNodeInformation(ctrl_ltras->hrqp,Irqp_node,node));

switch(node->boolean_operator.name) {
  case DOgBooleanNil:
    IFE(LtrasFcbLeaf(ctrl_ltras,input,output,Irqp_node));
    break;
  case DOgBooleanAnd:
    IFE(LtrasFcbAnd(ctrl_ltras,input,output,Irqp_node));
    break;
  case DOgBooleanOr:
    IFE(LtrasFcbOr(ctrl_ltras,input,output,Irqp_node));
    break;
  case DOgBooleanExcept:
    sprintf(erreur,"LtrasFcbSubtree: except node is not possible");
    OgErr(ctrl_ltras->herr,erreur); DPcErr;
    break;
  }

DONE;
}





/*
 * We are now at the module level, which can have arguments
 * or not, but we need to look at the function name.
*/

static int LtrasFcbLeaf(struct og_ctrl_ltras *ctrl_ltras
  ,struct og_ltra_trfs *input, struct og_ltra_trfs **output,int Irqp_node)
{
struct og_ltra_module_input cmodule_input,*module_input=&cmodule_input;
int ifullmodule; unsigned char fullmodule[DPcPathSize];
int argc; char *argv[DOgLtrasFlowChartMaxNbArgvs];
struct og_rqp_node crqp_node,*rqp_node=&crqp_node;
int ibuffer; unsigned char buffer[DPcPathSize];
int i,c,start,end,length,found_module;
intptr_t Inode; struct node *node;
struct module *module;

*output=0;

module_input->argv=argv;
IFE(OgRqpNodeInformation(ctrl_ltras->hrqp,Irqp_node,rqp_node));
IFE(OgUniToCp(rqp_node->name_length,rqp_node->name,DPcPathSize,&ifullmodule,fullmodule,DOgCodePageUTF8,0,0));
/** Calculating argc and argv **/
for (start=0,end=0,argc=0,i=0; !end; i++) {
  if (i>=ifullmodule) { c=')'; end=1; }
  else c=fullmodule[i];
  if (c=='(' || c==',' || c==')') {
    fullmodule[i]=0; OgTrimString(fullmodule+start,fullmodule+start);
    length=strlen(fullmodule+start);
    if (length > 0) argv[argc++]=fullmodule+start;
    start=i+1;
    }
  }
module_input->argc=argc;

if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleFlowChart) {
  IFE(OgUniToCp(rqp_node->name_length,rqp_node->name,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
    , "\nLtrasFcbLeaf: input ltras for module '%s':", buffer);
  IFE(OgLtrasTrfsLog(ctrl_ltras,input));
  }
if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleCalls) {
  IFE(OgUniToCp(rqp_node->name_length,rqp_node->name,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog , "LtrasFcbLeaf from '%s': starting module '%s'", ctrl_ltras->caller_label, buffer);
  }

/** For the moment we just suppose the flowchart is just one module **/
  for (found_module = 0, i = 0; i < ctrl_ltras->ModuleUsed; i++)
  {
    module = ctrl_ltras->Module + i;
    IFn(module->module) continue;
    if (Ogstricmp(ctrl_ltras->Ba + module->start_name, argv[0])) continue;
    module_input->handle = module->handle;
    module_input->id = i;
    IFn(input->TrfUsed) *output = input;
    else
    {
      ogint64_t elapsed = 0;
      IFE(module->module(module_input, input, output, &elapsed));
      module->nb_requests++;
      module->elapsed += elapsed;
    }
    found_module = 1;
    break;
  }
if (!found_module) {
  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleCalls) {
    IFE(OgUniToCp(rqp_node->name_length,rqp_node->name,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      , "LtrasFcbLeaf: could not find module '%s'", buffer);
  }
  /** This transformation is then made into the 'nil' transformation **/
  *output=input;
  }

IFE(Inode=LtrasAllocNode(ctrl_ltras,&node));
node->Irqp_node = Irqp_node; node->trfs = *output;
IFE(OgRqpNodeAddStructure(ctrl_ltras->hrqp,Irqp_node,(void *)Inode));

if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleFlowChart) {
  IFE(OgUniToCp(rqp_node->name_length,rqp_node->name,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
    , "LtrasFcbLeaf: output ltras for module '%s':", buffer);
  IFE(OgLtrasTrfsLog(ctrl_ltras,*output));
  }
if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleCalls) {
  IFE(OgUniToCp(rqp_node->name_length,rqp_node->name,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog , "LtrasFcbLeaf from '%s': finished module '%s'", ctrl_ltras->caller_label, buffer);
  }

DONE;
}




/*
 * 'and' corresponds to '/' thus a parallel calculation, thus
 * the loop on the node uses always the same input trfs
 * and all trfs are merged in a new trf.
*/

static int LtrasFcbAnd(struct og_ctrl_ltras *ctrl_ltras
  ,struct og_ltra_trfs *input, struct og_ltra_trfs **output,int Irqp_node)
{
struct og_ltra_trfs *trfs,*mother_trfs=0;
struct og_rqp_node crqp_node,*rqp_node=&crqp_node;
struct og_rqp_node cson,*son=&cson;
intptr_t Inode; struct node *node;
int i,first=1;

IFE(OgRqpNodeInformation(ctrl_ltras->hrqp,Irqp_node,rqp_node));

trfs=input;
/** This loop can be parallelized, at least the call to LtrasFcbSubtree **/
for (i=rqp_node->son; i>=0; i=son->next) {
  IFE(LtrasFcbSubtree(ctrl_ltras,input,&trfs,i));
  IFE(Inode=LtrasAllocNode(ctrl_ltras,&node));
  node->Irqp_node = i; node->trfs = trfs;
  IFE(OgRqpNodeAddStructure(ctrl_ltras->hrqp,i,(void *)Inode));
  if (first) {
    IFE(OgLtrasTrfsDuplicate(ctrl_ltras,trfs,&mother_trfs));
    first=0;
    }
  else {
    IFE(OgLtrasTrfsMerge(ctrl_ltras,trfs,mother_trfs));
    }
  IFE(OgRqpNodeInformation(ctrl_ltras->hrqp,i,son));
  if (trfs != input) {
    IFE(OgLtrasTrfsDestroy(ctrl_ltras,trfs));
    }
  }

IFE(Inode=LtrasAllocNode(ctrl_ltras,&node));
node->Irqp_node = Irqp_node; node->trfs = mother_trfs;
IFE(OgRqpNodeAddStructure(ctrl_ltras->hrqp,Irqp_node,(void *)Inode));
*output=mother_trfs;

DONE;
}




/*
 * 'or' corresponds to '-' thus a sequential calculation, thus
 * the loop on the node passes the trf from one
 * son to another.
*/

static int LtrasFcbOr(struct og_ctrl_ltras *ctrl_ltras
  ,struct og_ltra_trfs *input, struct og_ltra_trfs **output,int Irqp_node)
{
struct og_ltra_trfs *trfs,*ntrfs;
struct og_rqp_node crqp_node,*rqp_node=&crqp_node;
struct og_rqp_node cson,*son=&cson;
intptr_t Inode; struct node *node;
int i;

IFE(OgRqpNodeInformation(ctrl_ltras->hrqp,Irqp_node,rqp_node));

trfs=input;
for (i=rqp_node->son; i>=0; i=son->next) {
  IFE(LtrasFcbSubtree(ctrl_ltras,trfs,&ntrfs,i));
  IFE(Inode=LtrasAllocNode(ctrl_ltras,&node));
  node->Irqp_node = i; node->trfs = ntrfs;
  IFE(OgRqpNodeAddStructure(ctrl_ltras->hrqp,i,(void *)Inode));
  IFE(OgRqpNodeInformation(ctrl_ltras->hrqp,i,son));
  if (trfs != ntrfs && trfs != input) {
    IFE(OgLtrasTrfsDestroy(ctrl_ltras,trfs));
    }
  trfs=ntrfs;
  }

IFE(OgRqpNodeAddStructure(ctrl_ltras->hrqp,Irqp_node,(void *)Inode));
*output=ntrfs;

DONE;
}


