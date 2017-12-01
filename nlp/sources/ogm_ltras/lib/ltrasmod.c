/*
 *  Handling of module structure
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev: November 2009
 *  Version 1.0
*/
#include "ogm_ltras.h"



int LtrasAddPluginModule(struct og_ctrl_ltras *ctrl_ltras,unsigned char *name,unsigned char *function_name,unsigned char *library_name)
{
unsigned char flush_function_name[DPcPathSize];
unsigned char init_function_name[DPcPathSize];
int ilibname; char libname[DPcPathSize];
int Imodule; struct module *module;
char erreur[DOgErrorSize];
int iname=strlen(name);
DOgHinstance hlib;
#if (DPcSystem == DPcSystemUnix)
char *sys_erreur;
#else
#if (DPcSystem == DPcSystemWin32)
#endif
char sys_erreur[DOgErrorSize];
#endif


sprintf(init_function_name,"%sInit",function_name);
sprintf(flush_function_name,"%sFlush",function_name);

IFE(Imodule=LtrasAllocModule(ctrl_ltras,&module));
module->start_name = ctrl_ltras->BaUsed;
IFE(LtrasAppendBa(ctrl_ltras,iname,name));
ctrl_ltras->Ba[ctrl_ltras->BaUsed++]=0;
module->length_name=iname;

strcpy(libname,library_name);

#if (DPcSystem == DPcSystemUnix)
/* Normally we should not add .so at the end of the lib name
 * because we want the conf file to be independant from the plateform
 * but if the .so is already there, it is fine */
ilibname=strlen(libname);
if (Ogmemicmp(libname+ilibname-3,".so",3)) {
  strcpy(libname+ilibname,".so");
  }
IFn(hlib = dlopen(libname, RTLD_LAZY)) {
  sys_erreur = dlerror(); IFn(sys_erreur) sys_erreur="no error found";
  sprintf(erreur,"LtrasAddPluginModule: Impossible to load library '%s': %s", libname, sys_erreur);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
  }
// Necessary only for a dlclose call
ctrl_ltras->hmodules_lib = hlib;

module->init = dlsym(hlib, init_function_name);
IFn(module->init) {
  sys_erreur = dlerror(); IFn(sys_erreur) sys_erreur="no error found";
  sprintf(erreur, "LtrasAddPluginModule: Impossible to load %s: %s",init_function_name,sys_erreur);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
  }
module->module = dlsym(hlib, function_name);
IFn(module->module) {
  sys_erreur = dlerror(); IFn(sys_erreur) sys_erreur="no error found";
  sprintf(erreur, "LtrasAddPluginModule: Impossible to load %s: %s",function_name,sys_erreur);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
  }
module->flush = dlsym(hlib, flush_function_name);
IFn(module->init) {
  sys_erreur = dlerror(); IFn(sys_erreur) sys_erreur="no error found";
  sprintf(erreur, "LtrasAddPluginModule: Impossible to load %s: %s",flush_function_name,sys_erreur);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
  }

#else
#if (DPcSystem == DPcSystemWin32)
/* Normally we should not add .dll at the end of the lib name
 * because we want the conf file to be independant from the plateform
 * but if the .dll is already there, it is fine */
ilibname=strlen(libname);
if (Ogmemicmp(libname+ilibname-4,".dll",4)) {
  strcpy(libname+ilibname,".dll");
  }
else strcpy(libname,library_name);
IFn(hlib=LoadLibrary(libname)) {
  OgSysErrMes(OgSysErr(),DOgErrorSize/2,sys_erreur);
  sprintf(erreur, "LtrasAddPluginModule: Impossible to load library '%s': %s", libname,sys_erreur);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
  }

(FARPROC)(module->init) = GetProcAddress(hlib,init_function_name);
IFn(module->init) {
  OgSysErrMes(OgSysErr(),DOgErrorSize/2,sys_erreur);
  sprintf(erreur, "LtrasAddPluginModule: Impossible to load %s: %s",init_function_name,sys_erreur);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
  }
(FARPROC)(module->module) = GetProcAddress(hlib,function_name);
IFn(module->module) {
  OgSysErrMes(OgSysErr(),DOgErrorSize/2,sys_erreur);
  sprintf(erreur, "LtrasAddPluginModule: Impossible to load %s: %s",function_name,sys_erreur);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
  }
(FARPROC)(module->flush) = GetProcAddress(hlib,flush_function_name);
IFn(module->flush) {
  OgSysErrMes(OgSysErr(),DOgErrorSize/2,sys_erreur);
  sprintf(erreur, "LtrasAddPluginModule: Impossible to load %s: %s",flush_function_name,sys_erreur);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
  }

#endif
#endif

DONE;
}







int LtrasAllocModule(struct og_ctrl_ltras *ctrl_ltras,struct module **pmodule)
{
char erreur[DOgErrorSize];
struct module *module = 0;
int i=ctrl_ltras->ModuleNumber;

beginLtrasAllocModule:

if (ctrl_ltras->ModuleUsed < ctrl_ltras->ModuleNumber) {
  i = ctrl_ltras->ModuleUsed++;
  }

if (i == ctrl_ltras->ModuleNumber) {
  unsigned a, b; struct module *og_module;

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      , "LtrasAllocModule: max Module number (%d) reached"
      , ctrl_ltras->ModuleNumber);
    }
  a = ctrl_ltras->ModuleNumber; b = a + (a>>2) + 1;
  IFn(og_module=(struct module *)malloc(b*sizeof(struct module))) {
    sprintf(erreur,"LtrasAllocModule: malloc error on Module");
    OgErr(ctrl_ltras->herr,erreur); DPcErr;
    }

  memcpy( og_module, ctrl_ltras->Module, a*sizeof(struct module));
  DPcFree(ctrl_ltras->Module); ctrl_ltras->Module = og_module;
  ctrl_ltras->ModuleNumber = b;

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceMemory) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      , "LtrasAllocModule: new Module number is %d\n", ctrl_ltras->ModuleNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocModule: ModuleNumber reached (%d)",ctrl_ltras->ModuleNumber);
  OgErr(ctrl_ltras->herr,erreur); DPcErr;
#endif

  goto beginLtrasAllocModule;
  }

module = ctrl_ltras->Module + i;
memset(module,0,sizeof(struct module));

if (pmodule) *pmodule = module;
return(i);
}


