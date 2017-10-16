/*
 *  Copie d'un automate
 *  Copyright (c) 2013 Pertimm by Patrick Constant
 *  Dev : April 2014
 *  Version 1.0
 */

#include "ogm_aut.h"

/*
 * Copies only the aut section.
 */

PUBLIC(int) OgAutCopy(void *hfrom, void *hto)
{
  struct og_ctrl_aut *ctrl_aut_from = (struct og_ctrl_aut *) hfrom;
  struct og_ctrl_aut *ctrl_aut_to = (struct og_ctrl_aut *) hto;
  char erreur[DOgErrorSize];
  size_t size;

  ctrl_aut_to->StateNumber = ctrl_aut_from->StateNumber;
  ctrl_aut_to->StateUsed = ctrl_aut_from->StateUsed;
  ctrl_aut_to->StartFreeState = ctrl_aut_from->StartFreeState;

  if (ctrl_aut_from->State)
  {
    DPcFree(ctrl_aut_to->State);
    size = ctrl_aut_from->StateNumber * sizeof(struct state);
    IFn(ctrl_aut_to->State=(struct state *)malloc(size))
    {
      sprintf(erreur, "OgAutCopy (from %s to %s): malloc error on State with size %ld", ctrl_aut_from->name,
          ctrl_aut_to->name, size);
      OgErr(ctrl_aut_to->herr, erreur);
      DPcErr;
    }
    memcpy(ctrl_aut_to->State, ctrl_aut_from->State, size);
  }
  else
  {
    ctrl_aut_to->State = 0;
  }

  if (ctrl_aut_from->FreeState)
  {
    DPcFree(ctrl_aut_to->FreeState);
    size = ctrl_aut_from->StateNumber * sizeof(struct free_state);
    IFn(ctrl_aut_to->FreeState=(struct free_state *)malloc(size))
    {
      sprintf(erreur, "OgAutCopy (from %s to %s): malloc error on FreeState with size %ld", ctrl_aut_from->name,
          ctrl_aut_to->name, size);
      OgErr(ctrl_aut_to->herr, erreur);
      DPcErr;
    }
    memcpy(ctrl_aut_to->FreeState, ctrl_aut_from->FreeState, size);
  }
  else
  {
    ctrl_aut_to->FreeState = 0;
  }

  DONE;
}

