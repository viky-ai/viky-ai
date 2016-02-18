/*
 *  Handling of the 'state' heap.
 *  Copyright (c) 2000-2006 Stateertimm by Stateatrick Constant
 *  Dev : May 2000, February 2006
 *  Version 1.1
*/
#include "ogm_aut.h"




/*
 *  We allocate the free heap only when a delete of a state has been requested.
 *  Most of the time, we won't need to delete words from an automaton.
 *  in the genau implementation, this was manually controlled, but
 *  this is not needed at all.
*/

int FreeState(ctrl_aut,Istate)
struct og_ctrl_aut *ctrl_aut;
int Istate;
{
char erreur[DOgErrorSize];
IFn(ctrl_aut->FreeState) {
  if (ctrl_aut->loginfo->trace & DOgAutTraceMemory) {
    OgMessageLog( DOgMlogInLog, ctrl_aut->loginfo->where, 0
      , "FreeState (%s): allocating heap for delete State elements",ctrl_aut->name);
    }
  IFn(ctrl_aut->FreeState=(struct free_state *)malloc(ctrl_aut->StateNumber*sizeof(struct free_state))) {
    sprintf(erreur,"FreeState (%s): malloc error on FreeState",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); return(0);
    }
  }
ctrl_aut->FreeState[Istate].nx_free = ctrl_aut->StartFreeState;
ctrl_aut->StartFreeState = Istate;
DONE;
}





int AllocState(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
struct state *state;
char erreur[DOgErrorSize];
int i = ctrl_aut->StateNumber;

beginAllocState:

if (ctrl_aut->StartFreeState == (-1)) {
  if (ctrl_aut->StateUsed < ctrl_aut->StateNumber) {
    /** zero cell is forbidden because zero value of index is used **/
    IFn(ctrl_aut->StateUsed) {
      state = ctrl_aut->State + 0;
      memset(state,0,sizeof(struct state));
      ctrl_aut->StateUsed++;
      }
    i = ctrl_aut->StateUsed++;
    }
  }
else {
  i = ctrl_aut->StartFreeState;
  ctrl_aut->StartFreeState = ctrl_aut->FreeState[i].nx_free;
  }

if (i == ctrl_aut->StateNumber) {
  unsigned a, b; struct state *og_state;

  if (ctrl_aut->loginfo->trace & DOgAutTraceMemory) {
    OgMessageLog( DOgMlogInLog, ctrl_aut->loginfo->where, 0
                , "AllocState (%s): max State number (%d) reached"
                ,ctrl_aut->name , ctrl_aut->StateNumber);
    }
  a = ctrl_aut->StateNumber; b = a + (a>>2) + 1;
  IFn(og_state=(struct state *)malloc(b*sizeof(struct state))) {
    sprintf(erreur,"AllocState (%s): malloc error on State (%ld bytes)",ctrl_aut->name,b*sizeof(struct state));
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
  memcpy( og_state, ctrl_aut->State, a*sizeof(struct state));
  DPcFree(ctrl_aut->State); ctrl_aut->State = og_state;
  ctrl_aut->StateNumber = b;

  IFx(ctrl_aut->FreeState) {
    struct free_state *og_free_state;
    IFn(og_free_state=(struct free_state *)malloc(b*sizeof(struct free_state))) {
      sprintf(erreur,"AllocState (%s): malloc error on FreeState (%ld bytes)",ctrl_aut->name,b*sizeof(struct free_state));
      OgErr(ctrl_aut->herr,erreur); DPcErr;
      }
    memcpy( og_free_state, ctrl_aut->FreeState, a*sizeof(struct free_state));
    DPcFree(ctrl_aut->FreeState); ctrl_aut->FreeState = og_free_state;
    }

  if (ctrl_aut->loginfo->trace & DOgAutTraceMemory) {
    OgMessageLog( DOgMlogInLog, ctrl_aut->loginfo->where, 0
                , "AllocState (%s): new State number is %d\n",ctrl_aut->name , ctrl_aut->StateNumber);
    }

  if (ctrl_aut->StartFreeState != (-1)) {
    sprintf(erreur,"AllocState (%s): ctrl_aut->StartFreeState != DOgNMax",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocState (%s): StateNumber reached (%d)",ctrl_aut->name,ctrl_aut->StateNumber);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
#endif

  goto beginAllocState;
  }


if (i == ctrl_aut->StateNumber) {
  sprintf(erreur,"AllocState (%s): StateNumber reached (%d)",ctrl_aut->name,ctrl_aut->StateNumber);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

state = ctrl_aut->State + i;
memset(state,0,sizeof(struct state));

return(i);
}



