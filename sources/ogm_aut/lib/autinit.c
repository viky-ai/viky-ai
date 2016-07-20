/*
 *  Initialisation de tout le module de gestion des automates.
 *  Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *  Dev : Mars,Juillet,Septembre 1996, Janvier 1997, February 2006
 *  Version 1.4
*/
#include "ogm_aut.h"




PUBLIC(void *) OgAutInit(param)
struct og_aut_param *param;
{
char erreur[DOgErrorSize];
struct og_ctrl_aut *ctrl_aut;

IFn(ctrl_aut=(struct og_ctrl_aut *)malloc(sizeof(struct og_ctrl_aut))) {
  sprintf(erreur,"OgAutInit: malloc error on ctrl_aut");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_aut,0,sizeof(struct og_ctrl_aut));

ctrl_aut->herr = param->herr;
ctrl_aut->hmutex = param->hmutex;
ctrl_aut->cloginfo = param->loginfo;
ctrl_aut->loginfo = &ctrl_aut->cloginfo;
snprintf(ctrl_aut->name, DOgAutNameSize, "%s", param->name);

ctrl_aut->StateUsed = 0;
ctrl_aut->StartFreeState = (-1);
if (param->state_number > 0) {
  ctrl_aut->StateNumber = param->state_number;
  IFn(ctrl_aut->State=(struct state *)malloc(ctrl_aut->StateNumber*sizeof(struct state))) {
    sprintf(erreur,"OgAutInit (%s): malloc error on State",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); return(0);
    }
  }
ctrl_aut->LstminNumber=DOgLstminNumber;
IFn(ctrl_aut->Lstmin=(struct lstmin *)malloc(ctrl_aut->LstminNumber*sizeof(struct lstmin))) {
  sprintf(erreur,"OgAutInit (%s): malloc error on Lstmin",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); return(0);
  }
return((void *)ctrl_aut);
}




PUBLIC(int) OgAutSize(void *handle)
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
return(ctrl_aut->StateUsed);
}




PUBLIC(int) OgAutUsed(void *handle)
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
return(ctrl_aut->StateUsed);
}




PUBLIC(int) OgAutNumber(void *handle)
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
return(ctrl_aut->StateNumber);
}




PUBLIC(int) OgAutFree(handle)
void *handle;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
DPcFree(ctrl_aut->State); ctrl_aut->StateNumber=0; ctrl_aut->StateUsed=0;
DPcFree(ctrl_aut->FreeState);
DONE;
}





PUBLIC(int) OgAufFree(handle)
void *handle;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
DPcFree(ctrl_aut->Fstate);
DPcFree(ctrl_aut->Gstate);
DONE;
}





PUBLIC(char *) OgAutBanner()
{
return(DOgAutBanner);
}





PUBLIC(int) OgAutResize(void *handle, int max_state_number)
{
  struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *) handle;

  size_t preview_size = ctrl_aut->StateNumber * sizeof(struct state);
  size_t preview_state_number = ctrl_aut->StateNumber;

  ctrl_aut->StateUsed = 0;
  if (ctrl_aut->StateNumber > max_state_number)
  {
    DPcFree(ctrl_aut->State);
    ctrl_aut->StateNumber = max_state_number;
    size_t new_size = ctrl_aut->StateNumber * sizeof(struct state);
    IFn(ctrl_aut->State = (struct state *) malloc(new_size))
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "OgAutResize on '%s': malloc error on State (%zu bytes)", ctrl_aut->name, new_size);
      OgErr(ctrl_aut->herr, erreur);
      DPcErr;
    }

    if (ctrl_aut->loginfo->trace & DOgAutTraceMemory)
    {
      OgMessageLog(DOgMlogInLog, ctrl_aut->loginfo->where, 0,
          "OgAutResize on '%s': reducing State size from %ld to %ld states (from %zu to %zu bytes)", ctrl_aut->name,
          preview_state_number, max_state_number, preview_size, new_size);
    }

  }
  DONE;
}

