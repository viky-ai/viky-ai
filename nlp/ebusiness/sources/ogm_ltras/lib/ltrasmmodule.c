/*
 * ltrasmmodule.c
 *
 *  Created on: 29 oct. 2014
 *      Author: caroline
 */
/*
 *  Handling score structures
 *  Copyright (c) 2009-2010 Pertimm by Patrick Constant
 *  Dev: November 2009, January 2010
 *  Version 1.0
*/
#include "ogm_ltras.h"


static int LtrasTrfAllocModule(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_module **pmodule);



int LtrasAddModule(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_trf *trf, int module_id)
{
  int Imodule;
  struct og_ltra_module *module;

  IFE(Imodule = LtrasTrfAllocModule(ctrl_ltras, trfs, &module));
  if (trf->start_module < 0)
  {
    trf->start_module = Imodule;
    trf->nb_modules = 1;
  }
  else trf->nb_modules++;
  module->module_id = module_id;
  DONE;
}


static int LtrasTrfAllocModule(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, struct og_ltra_module **pmodule)
{
char erreur[DOgErrorSize];
struct og_ltra_module *module = 0;
int i=trfs->ModuleNumber;

beginAllocModule:

if (trfs->ModuleUsed < trfs->ModuleNumber) {
  i = trfs->ModuleUsed++;
  }

if (i == trfs->ModuleNumber) {
  unsigned a, b; struct og_ltra_module *og_trf;

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      , "AllocModule: max Module number (%d) reached"
      , trfs->ModuleNumber);
    }
  a = trfs->ModuleNumber; b = a + (a>>2) + 1;
  IFn(og_trf=(struct og_ltra_module *)malloc(b*sizeof(struct og_ltra_module))) {
    sprintf(erreur,"AllocModule: malloc error on Module");
    OgErr(ctrl_ltras->herr,erreur); DPcErr;
    }

  memcpy( og_trf, trfs->Module, a*sizeof(struct og_ltra_module));
  DPcFree(trfs->Module); trfs->Module = og_trf;
  trfs->ModuleNumber = b;

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      , "AllocModule: new Module number is %d\n", trfs->ModuleNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocModule: ModuleNumber reached (%d)",trfs->ModuleNumber);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
#endif

  goto beginAllocModule;
  }

module = trfs->Module + i;
memset(module,0,sizeof(struct og_ltra_module));

if (pmodule) *pmodule = module;
return(i);
}


